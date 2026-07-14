/**
 * @file    bsp_track.h
 * @brief   循迹模块 - 加权质心 + PD 控制
 *          同时兼容 有MCU(数字GPIO) / 无MCU(模拟ADC) 灰度传感器
 * 
 * 使用方法:
 *   Phase 1 - 有MCU传感器(数字GPIO): 默认编译即可
 *   Phase 2 - 无MCU传感器(模拟ADC): 取消下方 USE_ANALOG_SENSOR 注释
 *             并实现 gray_sensor_analog.c 的 Get_Analog_value()
 */
#ifndef __BSP_TRACK_H__
#define __BSP_TRACK_H__

#include "ti_msp_dl_config.h"
#include "board.h"

/* ======================== 传感器类型选择 ======================== */
// 当无MCU模拟传感器到货后，取消下面这行的注释即可切换
// #define USE_ANALOG_SENSOR

/* ======================== GPIO读取宏(有MCU传感器) ======================== */
// 8路传感器从左到右: L4(最左) L3 L2 L1 R1 R2 R3 R4(最右)
// 逻辑: 读到低电平(0V)=检测到黑线 → 宏=1
#define TRACK_READ(PORT, PIN)  \
    ( !( (DL_GPIO_readPins(PORT, PIN) > 0) ? 1 : DL_GPIO_readPins(PORT, PIN) ) )

#define L4  TRACK_READ(TRACK_S1_PORT, TRACK_S1_PIN)
#define L3  TRACK_READ(TRACK_S2_PORT, TRACK_S2_PIN)
#define L2  TRACK_READ(TRACK_S3_PORT, TRACK_S3_PIN)
#define L1  TRACK_READ(TRACK_S4_PORT, TRACK_S4_PIN)
#define R1  TRACK_READ(TRACK_S5_PORT, TRACK_S5_PIN)
#define R2  TRACK_READ(TRACK_S6_PORT, TRACK_S6_PIN)
#define R3  TRACK_READ(TRACK_S7_PORT, TRACK_S7_PIN)
#define R4  TRACK_READ(TRACK_S8_PORT, TRACK_S8_PIN)

/* ======================== 外部接口函数 ======================== */
void track_init(void);
void track_control(void);

/* 调参辅助（通过串口/按键在运行时修改） */
void track_set_speed(int base_speed);
void track_set_pid(float kp, float ki, float kd);

#endif /* __BSP_TRACK_H__ */
