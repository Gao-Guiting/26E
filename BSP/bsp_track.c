/**
 * @file    bsp_track.c
 * @brief   循迹核心: 加权位置 + PD控制器 + 丢线保护 + 差速转向
 */

#include "bsp_track.h"
#include "bsp_motor.h"
#include "stdio.h"
#include <stdlib.h>

/* ======================== 运动参数 ======================== */
#define VMAX            90
#define VMIN            30

#define BASE_SPEED      1500
#define MAX_CORRECT     600

#define OUT_MAX         100
#define INTEGRAL_MAX    5
#define INTEGRAL_MIN   -5

/* ======================== 传感器权值 ======================== */
static const short weights[8] = {-7, -5, -3, -1, 1, 3, 5, 7};

/* ======================== PID参数 ======================== */
static int Kp_Yaw = 10;
static int Ki_Yaw = 0;
static int Kd_Yaw = 50;

/* ======================== PID状态 ======================== */
static int Integral_Yaw = 0;
static int LastErr_Yaw = 0;
static int Output_Yaw = 0;
static int result_angle = 0;
static int last_result_angle = 0;
static int lost_counter = 0;
static int Speed_Set = VMAX;

/* ======================== 传感器缓冲区 ======================== */
static unsigned short sensor_raw[8];
static unsigned short sensor_norm[8];

/* ======================== 校准值(模拟量用) ======================== */
static unsigned short white_calib[8] = {1155, 2009, 1528, 2344, 2320, 1757, 1721, 1420};
static unsigned short black_calib[8] = {67, 72, 72, 72, 73, 74, 73, 64};

/* ======================== 传感器读取层 ======================== */

static void read_sensor_raw(void)
{
#ifdef USE_ANALOG_SENSOR
    extern void Get_Analog_value(unsigned short *result);
    Get_Analog_value(sensor_raw);
#else
    unsigned char bits[8] = {L4, L3, L2, L1, R1, R2, R3, R4};
    for (int i = 0; i < 8; i++) {
        sensor_raw[i] = bits[i] ? 0 : 4096;
    }
#endif
}

static void normalize_sensor(void)
{
#ifdef USE_ANALOG_SENSOR
    for (int i = 0; i < 8; i++) {
        unsigned short v = sensor_raw[i];
        if (v <= black_calib[i])
            sensor_norm[i] = 0;
        else if (v >= white_calib[i])
            sensor_norm[i] = 4096;
        else {
            unsigned int r = white_calib[i] - black_calib[i];
            sensor_norm[i] = (r == 0) ? 2048
                           : (unsigned int)(v - black_calib[i]) * 4096 / r;
        }
    }
#else
    for (int i = 0; i < 8; i++)
        sensor_norm[i] = sensor_raw[i];
#endif
}

/* ======================== 加权质心 ======================== */

static int32_t calc_centroid(uint8_t field)
{
    int32_t wsum = 0, osum = 0;

    for (int i = 0; i < 8; i++) {
        int32_t b = field ? (4096 - sensor_norm[i]) : sensor_norm[i];
        wsum += 1024 * b * weights[i];
        osum += b;
    }

    if (osum != 0) {
        last_result_angle = wsum / osum;
        return last_result_angle;
    }
    return last_result_angle;
}

/* ======================== PD控制器 ======================== */

static void pid_yaw(float target)
{
    int err = (int)(target * 1024) - result_angle;

    Integral_Yaw += err;
    if (Integral_Yaw > INTEGRAL_MAX * 1024)
        Integral_Yaw = INTEGRAL_MAX * 1024;
    if (Integral_Yaw < INTEGRAL_MIN * 1024)
        Integral_Yaw = INTEGRAL_MIN * 1024;

    Output_Yaw = (Kp_Yaw * err) / 1024
               + (Kd_Yaw * (err - LastErr_Yaw)) / 1024
               + (Ki_Yaw * Integral_Yaw) / 1024;

    LastErr_Yaw = err;

    if (Output_Yaw > OUT_MAX)  Output_Yaw = OUT_MAX;
    if (Output_Yaw < -OUT_MAX) Output_Yaw = -OUT_MAX;
}

/* ======================== 丢线检测 ======================== */

static int check_lost(void)
{
    unsigned char b[8] = {L4, L3, L2, L1, R1, R2, R3, R4};
    int cnt = 0, left = 0, right = 0;

    for (int i = 0; i < 8; i++) {
        if (b[i]) cnt++;
    }
    left  = b[0] || b[1];
    right = b[6] || b[7];

    if (cnt == 0) {
        lost_counter++;
        if (lost_counter > 5) {
            Speed_Set = VMIN;
            Output_Yaw = (last_result_angle < 0) ? OUT_MAX : -OUT_MAX;
            return 1;
        }
        return 0;
    }
    lost_counter = 0;

    if (cnt <= 2) {
        if (left && !right) {
            Speed_Set = VMIN;
            Output_Yaw = OUT_MAX;
            return 1;
        }
        if (right && !left) {
            Speed_Set = VMIN;
            Output_Yaw = -OUT_MAX;
            return 1;
        }
    }
    return 0;
}

/* ======================== 速度自适应 ======================== */

static void speed_adapt(void)
{
    int abs_angle = abs(last_result_angle);
    int speed = VMAX - 60 * abs_angle / 7168;
    if (speed < VMIN) speed = VMIN;
    Speed_Set = speed;
}

/* ======================== 电机输出 ======================== */

void motorsWrite(int speedL, int speedR)
{
    if (speedR > 0) {
        DL_GPIO_setPins(MOTOR_BIN1_B20_PORT, MOTOR_BIN1_B20_PIN);
        DL_GPIO_clearPins(MOTOR_BIN2_B19_PORT, MOTOR_BIN2_B19_PIN);
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, abs(speedR), GPIO_PWM_MOTOR_C1_IDX);
    } else {
        DL_GPIO_setPins(MOTOR_BIN2_B19_PORT, MOTOR_BIN2_B19_PIN);
        DL_GPIO_clearPins(MOTOR_BIN1_B20_PORT, MOTOR_BIN1_B20_PIN);
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, abs(speedR), GPIO_PWM_MOTOR_C1_IDX);
    }

    if (speedL > 0) {
        DL_GPIO_setPins(MOTOR_AIN2_B24_PORT, MOTOR_AIN2_B24_PIN);
        DL_GPIO_clearPins(MOTOR_AIN1_A02_PORT, MOTOR_AIN1_A02_PIN);
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, abs(speedL), GPIO_PWM_MOTOR_C0_IDX);
    } else {
        DL_GPIO_setPins(MOTOR_AIN1_A02_PORT, MOTOR_AIN1_A02_PIN);
        DL_GPIO_clearPins(MOTOR_AIN2_B24_PORT, MOTOR_AIN2_B24_PIN);
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, abs(speedL), GPIO_PWM_MOTOR_C0_IDX);
    }
}

/* ======================== 外部接口 ======================== */

void track_init(void)
{
    printf("=== Track: Weighted-Position + PD ===\n");
#ifdef USE_ANALOG_SENSOR
    printf("Sensor: Analog (ADC)\n");
#else
    printf("Sensor: Digital (GPIO)\n");
#endif
    printf("Kp=%d  Ki=%d  Kd=%d\n", Kp_Yaw, Ki_Yaw, Kd_Yaw);
}

void track_control(void)
{
    /* 1-2: 读取 + 归一化 */
    read_sensor_raw();
    normalize_sensor();

    /* 3: 加权质心 */
    result_angle = calc_centroid(1);

    /* 4: PD */
    pid_yaw(0);

    /* 5: 速度自适应 */
    speed_adapt();

    /* 6: 丢线保护 */
    check_lost();

    /* 7: 差速输出 */
    int pwm_adj = Output_Yaw * MAX_CORRECT / OUT_MAX;
    int speedL = Speed_Set * BASE_SPEED / 100 - pwm_adj;
    int speedR = Speed_Set * BASE_SPEED / 100 + pwm_adj;

    if (speedL > 9999) speedL = 9999;
    if (speedL < -9999) speedL = -9999;
    if (speedR > 9999) speedR = 9999;
    if (speedR < -9999) speedR = -9999;

    motorsWrite(-speedL, speedR);

    last_result_angle = result_angle;
}

void track_set_pid(float kp, float ki, float kd)
{
    Kp_Yaw = (int)(kp * 100);
    Ki_Yaw = (int)(ki * 100);
    Kd_Yaw = (int)(kd * 100);
    printf("PID: Kp=%.2f Ki=%.2f Kd=%.2f\n", kp, ki, kd);
}
