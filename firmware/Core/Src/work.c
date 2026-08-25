/**
 * @file work_process.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "work.h"
#include "main.h"
#include "rtthread.h"
#include "save.h"
#include "motor.h"
#include "screen.h"
#include "pwr.h"
struct process_data_t process_data;
/**
 * @brief 处理状态切换
 * 
 * @param target_state 
 */
void process_switch(enum process_state_t target_state)
{
	switch (target_state){
		case PROCESS_STATE_INIT:
			break;

		case PROCESS_STATE_STADYING:
			process_data.time.stadying = 0;
			screen_main_page_switch(SCREEN_PAGE_REALTIME);
			break;

		case PROCESS_STATE_REVERING:
			motor_send_msg((enum motor_msg_type_t)process_data.station, MOTOR_DIR_FORWARD);
			screen_main_page_switch(SCREEN_PAGE_REVERSING);
			break;

		case PROCESS_STATE_STATION_INTERVAL:
			break;

		case PROCESS_STATE_STOP:
			break;
	}

	process_data.state = target_state;
}

/**
 * @brief 工作流程初始化
 * 
 */
void process_init(void)
{
	process_data.station = 1;
	process_data.time.reversing_count_down = save_set_data.rever_period_sum_s;
	process_switch(PROCESS_STATE_STADYING);
}

/**
 * @brief 正在待机中
 * 
 */
void process_stadying(void)
{
        process_data.time.stadying++;
	if (process_data.time.stadying >= save_set_data.rever_period_sum_s ||
		pressure_data.bar >= save_set_data.rever_bar_diff){
		process_data.time.stadying = 0;
		process_switch(PROCESS_STATE_REVERING);
	}
}

/**
 * @brief 正在反冲洗流程中
 * 
 */
void process_revering(void)
{
	// 反冲洗倒计时
	process_data.time.reversing_count_down--;
	if (process_data.time.reversing_count_down <= 0){

		// 反冲洗结束，关闭站点电磁阀
		process_data.time.reversing_count_down = save_set_data.rever_time_sum_s;
		motor_send_msg((enum motor_msg_type_t)process_data.station, MOTOR_DIR_OPEN);
		process_data.station++;

		// 判断是否需要切换到下一个站点
		if (process_data.station > save_set_data.station_num){
			process_data.station = 1;
			process_switch(PROCESS_STATE_STADYING);
			process_data.reverse_period_num++;
			if (process_data.reverse_period_num > 9999){
				process_data.reverse_period_num = 0;
			}
		} else {
			process_data.time.station_interval_count_down = PROCESS_STATION_INTERVAL;
			process_switch(PROCESS_STATE_STATION_INTERVAL);
			screen_main_page_switch(SCREEN_PAGE_STATION_INTERVAL);
		}
	}
}

/**
 * @brief 站点间隔中，默认5秒
 * 
 */
void process_station_interval(void)
{
	process_data.time.station_interval_count_down--;
	if (process_data.time.station_interval_count_down <= 0){
		process_data.time.station_interval_count_down = PROCESS_STATION_INTERVAL;
		process_switch(PROCESS_STATE_REVERING);
	}
}

void process_stop(void)
{
	process_data.station = 1;
	process_data.time.reversing_count_down = save_set_data.rever_period_sum_s;
	process_data.time.station_interval_count_down = PROCESS_STATION_INTERVAL;
	process_data.time.stadying = 0;
}

void process_handle(void)
{
	switch (process_data.state){
		case PROCESS_STATE_INIT:
			process_init();
			break;

		case PROCESS_STATE_STADYING:
			process_stadying();
			break;

		case PROCESS_STATE_REVERING:
			process_revering();
			break;

		case PROCESS_STATE_STATION_INTERVAL:
			process_station_interval();
			break;

		case PROCESS_STATE_STOP:
			process_stop();
			break;
	}
}

