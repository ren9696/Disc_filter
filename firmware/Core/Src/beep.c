/**
 * @file beep.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "beep.h"
#include "main.h"
#include "rtthread.h"
static struct rt_thread beep_tid;
#define BEEP_THREAD_STACK_SIZE 128
#define BEEP_THREAD_PRIORITY 20
#define BEEP_THREAD_TIMESLICE 20
static char beep_stack[BEEP_THREAD_STACK_SIZE];
static struct rt_messagequeue beep_mq;
#define BEEP_MSG_POOL_SIZE 16
static rt_uint8_t beep_msg_pool[BEEP_MSG_POOL_SIZE];
struct rt_timer beep_timer;
INIT_APP_EXPORT(beep_init);

void beep_send(enum beep_smg_type_e type)
{
	rt_mq_send(&beep_mq, &type, sizeof(enum beep_smg_type_e));
}

void beep_ctrl(uint8_t on)
{
	if (on){
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
	}
}

void beep_msg_handler(enum beep_smg_type_e type)
{
	switch(type){
		case BEEP_MSG_TYPE_1:
			//beep_ctrl(1);
			rt_timer_start(&beep_timer);
			break;
		default:
			break;
	}
}

void beep_entry(void *parameter)
{
	enum beep_smg_type_e msg_type;

	while(1){
		if (rt_mq_recv(&beep_mq, &msg_type, sizeof(enum beep_smg_type_e), RT_WAITING_FOREVER) == RT_EOK){
			beep_msg_handler(msg_type);
		}
	}
}

void beep_timer_callback(void *parameter)
{
        beep_ctrl(0);
}

int beep_init(void)
{
	rt_timer_init(&beep_timer, "beep_timer",
			beep_timer_callback,
			NULL, 
			50, 
			RT_TIMER_FLAG_ONE_SHOT); 

	rt_mq_init(&beep_mq,
			"ec_msg",
			&beep_msg_pool[0],             
			sizeof(enum beep_smg_type_e),
			BEEP_MSG_POOL_SIZE,        
			RT_IPC_FLAG_FIFO);      

	rt_thread_init(&beep_tid,
			"beep",
			beep_entry,
			RT_NULL,
			beep_stack,
			BEEP_THREAD_STACK_SIZE,
			BEEP_THREAD_PRIORITY, 
			BEEP_THREAD_TIMESLICE);
	rt_thread_startup(&beep_tid);
	return 0;
}
