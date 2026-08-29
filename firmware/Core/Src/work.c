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

#define WORK_THREAD_PRIORITY 10
#define WORK_THREAD_TIMESLICE 20
#define WORK_THREAD_STACK_SIZE 1024
static struct rt_thread work_tid;
static char work_stack[WORK_THREAD_STACK_SIZE];
INIT_APP_EXPORT(work_init);

static struct rt_messagequeue work_mq;
#define WORK_MSG_POOL_SIZE (sizeof(enum work_msg_type_t) * 16)
static rt_uint8_t work_msg_pool[WORK_MSG_POOL_SIZE];

void process_init(void);

/**
 * @brief 发送消息给工作线程
 * 
 * @param msg 
 * @return int 0:成功 -1:失败
 */
int work_send_msg(enum work_msg_type_t msg)
{
	if (rt_mq_send(&work_mq, &msg, sizeof(enum work_msg_type_t)) != RT_EOK){
		return -1;
	}
	return 0;
}

/**
 * @brief 工作线程：状态机唯一入口
 *        秒脉冲和按键命令共用消息队列，保证处理顺序一致
 * 
 * @param param 
 */
static void work_entry(void *param)
{
	enum work_msg_type_t msg;
	while (1){
		if (rt_mq_recv(&work_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK){
			switch (msg){
			case WORK_MSG_TYPE_SECOND:
				process_handle();
				break;

			case WORK_MSG_TYPE_STOP:
				process_switch(PROCESS_STATE_STOP);
				screen_main_page_switch(SCREEN_PAGE_SETING);
				screen_data.setting_page_index = SETING_PAGE_REVER_PERIOD;
				break;

			case WORK_MSG_TYPE_INIT:
				process_init();
				break;

			case WORK_MSG_TYPE_MANUAL:
				if (process_data.state == PROCESS_STATE_INIT){
					process_init();
				}
				process_switch(PROCESS_STATE_REVERING);
				break;

			default:
				break;
			}
		}
	}
}

int work_init(void)
{
	rt_mq_init(&work_mq,
			"work_msg",
			&work_msg_pool[0],
			sizeof(enum work_msg_type_t),
			WORK_MSG_POOL_SIZE,
			RT_IPC_FLAG_FIFO);

	rt_thread_init(&work_tid,
			"WORK",
			work_entry,
			RT_NULL,
			&work_stack[0],
			WORK_THREAD_STACK_SIZE,
			WORK_THREAD_PRIORITY,
			WORK_THREAD_TIMESLICE);

	rt_thread_startup(&work_tid);
	return 0;
}

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
	process_data.time.reversing_count_down = save_set_data.rever_time_sum_s;
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
			process_data.time.station_interval_count_down = PROCESS_STATION_INTERVAL;	// 当前站点间隔默认强制设置为5
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
	process_data.time.reversing_count_down = save_set_data.rever_time_sum_s;
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

