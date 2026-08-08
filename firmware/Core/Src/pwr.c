/**
 * @file pwr.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "pwr.h"
#include "main.h"
#include "rtthread.h"

/**
 * @brief 屏幕电源控制
 * 
 * @param power 1: 关闭屏幕电源
 *              0: 打开屏幕电源
 * 		低电平有效
 * @return int 
 */
int screen_power_ctrl(int power)
{
	if (power == 0){
		HAL_GPIO_WritePin(SCREEN_PWR_GPIO_Port, 
				  SCREEN_PWR_Pin, 
				  GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(SCREEN_PWR_GPIO_Port, 
				  SCREEN_PWR_Pin, 
				  GPIO_PIN_SET);
	}

	return 0;
}
