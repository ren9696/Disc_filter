/**
 * @file save.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef SAVE_H
#define SAVE_H
#include "stdint.h"

#define SAVE_FLASH_DEFAULT_REVER_PERIOD_H 	0
#define SAVE_FLASH_DEFAULT_REVER_PERIOD_M 	2
#define SAVE_FLASH_DEFAULT_REVER_TIME_M 	0
#define SAVE_FLASH_DEFAULT_REVER_TIME_S 	30
#define SAVE_FLASH_DEFAULT_REVER_BAR_DIFF 	20
#define SAVE_FLASH_DEFAULT_STATION_INTERVAL 	5
#define SAVE_FLASH_DEFAULT_STATION_NUM 		2
#define SAVE_FLASH_DEFAULT_MAIN_VALVE 		8
#define SAVE_FLASH_DEFAULT_ID 			0x5A5A5A5A

struct save_set_data_t{
	int rever_period_h;
	int rever_period_m;
	int rever_period_sum_s;

	int rever_time_m;
	int rever_time_s;
	int rever_time_sum_s;

	int rever_bar_diff;
	int station_interval;
	int station_num;
	int main_valve;	
	int save_flash_default_id;
};


enum save_msg_type_t{
	SAVE_MSG_TYPE_NONE,
	SAVE_MSG_TYPE_SAVE_ALL_DATA,
};

extern struct save_set_data_t save_set_data;

int save_init(void);
int save_send_msg(enum save_msg_type_t msg);
#endif
