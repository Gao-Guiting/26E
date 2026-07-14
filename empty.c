/*
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-26     LCKFB     80MHz-MSPM0G3507
 */

#include "board.h"
#include "stdio.h"
#include "bsp_motor.h"
#include "bsp_track.h"

int32_t motor_mode = MOTOR_TRACK;//0默认停止  1速度环  2位置环  3循迹


int main(void)
{
	board_init();
	
	motor_init(); //开启电机编码器引脚外部中断
	track_init(); //开启循迹模块初始化，该函数没有作用，只是提醒工程师有配置循迹模块

	while (1) 
	{         
		track_control();
		
	}
}

//initial