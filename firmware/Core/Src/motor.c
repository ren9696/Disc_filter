/**
 * @file motor.c
 * @author your name (you@domain.com)
 * @brief ¿ØÖÆµç´Å·§µÄ
 * @version 0.1
 * @date 2026-08-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "motor.h"
#include "main.h"
#include "rtthread.h"
#include "pwr.h"
#include "beep.h"

struct motor_data_t motor1;
struct motor_data_t motor2;
struct motor_data_t motor3;
struct motor_data_t motor4;
struct motor_data_t motor5;
struct motor_data_t motor6;
#define MOTOR_PLUS_MS 200
#if 1
struct rt_timer motor_timer1;
struct rt_timer motor_timer2;
struct rt_timer motor_timer3;
struct rt_timer motor_timer4;
struct rt_timer motor_timer5;
struct rt_timer motor_timer6;
#define MOTOR_THREAD_PRIORITY 		12
#define MOTOR_THREAD_TIMESLICE 		20
#define MOTOR_THREAD_STACK_SIZE 	1024
static struct rt_thread motor_tid;
static char motor_stack[MOTOR_THREAD_STACK_SIZE];
INIT_APP_EXPORT(motor_init);
static struct rt_messagequeue motor_mq;
#define MOTOR_MSG_POOL_SIZE 	(sizeof(struct motor_msg_t) * 6)
static rt_uint8_t motor_msg_pool[MOTOR_MSG_POOL_SIZE];
#endif
static struct motor_msg_t motor_glb_send_msg;


int motor_send_msg(enum motor_msg_type_t msg_type, enum motor_opt_t opt)
{
	motor_glb_send_msg.type = msg_type;
	motor_glb_send_msg.opt  = opt;

	if (rt_mq_send(&motor_mq, &motor_glb_send_msg, sizeof(struct motor_msg_t)) != RT_EOK){
		return -1;	
	}
	return 0;
}

int motor_hard_info_forward_register(struct motor_data_t *motor, GPIO_TypeDef *port, uint16_t pin)
{
        if (motor == RT_NULL || port == RT_NULL){
                return -1;
        }

	motor->hard_info_forward.port = port;
	motor->hard_info_forward.pin = pin;
	return 0;
}

int motor_hard_info_back_register(struct motor_data_t *motor, GPIO_TypeDef *port, uint16_t pin)
{
        if (motor == RT_NULL || port == RT_NULL){
                return -1;
        }

	motor->hard_info_back.port = port;
	motor->hard_info_back.pin = pin;
	return 0;
}

void motor_hard_ctrl(struct motor_data_t *motor_data, enum motor_opt_t opt)
{
	switch (opt){
	case MOTOR_DIR_FORWARD:
		motor_power_ctrl(1);
		motor_data->motor_state = MOTOR_DIR_FORWARD;
		HAL_GPIO_WritePin(motor_data->hard_info_forward.port, 
				motor_data->hard_info_forward.pin, 
				GPIO_PIN_SET);
		HAL_GPIO_WritePin(motor_data->hard_info_back.port,
				motor_data->hard_info_back.pin,
				GPIO_PIN_RESET);
		motor_timer_start(motor_data);
		break;

	case MOTOR_DIR_BACKWARD:
		motor_power_ctrl(1);
		motor_data->motor_state = MOTOR_DIR_BACKWARD;
		HAL_GPIO_WritePin(motor_data->hard_info_forward.port,
				motor_data->hard_info_forward.pin,
				GPIO_PIN_RESET);
		HAL_GPIO_WritePin(motor_data->hard_info_back.port,
				motor_data->hard_info_back.pin,
				GPIO_PIN_SET);
		motor_timer_start(motor_data);
		break;

	case MOTOR_DIR_STOP:
		motor_data->motor_state = MOTOR_DIR_STOP;
		HAL_GPIO_WritePin(motor_data->hard_info_forward.port,
				motor_data->hard_info_forward.pin,
				GPIO_PIN_SET);
		HAL_GPIO_WritePin(motor_data->hard_info_back.port,
				motor_data->hard_info_back.pin,
				GPIO_PIN_SET);
		motor_power_ctrl(0);
		break;

	case MOTOR_DIR_OPEN:
		motor_data->motor_state = MOTOR_DIR_OPEN;
		HAL_GPIO_WritePin(motor_data->hard_info_forward.port,
				motor_data->hard_info_forward.pin,
				GPIO_PIN_RESET);
		HAL_GPIO_WritePin(motor_data->hard_info_back.port,
				motor_data->hard_info_back.pin,
				GPIO_PIN_RESET);
		motor_power_ctrl(0);
		break;
	default:
		break;
	}
}
 
int motor_ctrl(enum motor_msg_type_t msg_type, enum motor_opt_t opt)
{
        switch(msg_type){
	case MOTOR_MSG_TYPE_1:
		motor_hard_ctrl(&motor1, opt);
		break;
	case MOTOR_MSG_TYPE_2:
		motor_hard_ctrl(&motor2, opt);
		break;
	case MOTOR_MSG_TYPE_3:
		motor_hard_ctrl(&motor3, opt);
		break;
	case MOTOR_MSG_TYPE_4:
		motor_hard_ctrl(&motor4, opt);
		break;
	case MOTOR_MSG_TYPE_5:
		motor_hard_ctrl(&motor5, opt);
		break;
	case MOTOR_MSG_TYPE_6:
		motor_hard_ctrl(&motor6, opt);
		break;
	default:
		break;
        }
	beep_send(BEEP_MSG_TYPE_1);
	return 0;
}

void motor_entry(void *parameter)
{
	struct motor_msg_t msg;
	
	motor_power_ctrl(1);
	
	motor1.motor_id = MOTOR_MSG_TYPE_1;
	motor_hard_info_forward_register(&motor1, M1_FI_GPIO_Port, M1_FI_Pin);
	motor_hard_info_back_register(&motor1, 	  M1_BI_GPIO_Port, M1_BI_Pin);

	motor2.motor_id = MOTOR_MSG_TYPE_2;
	motor_hard_info_forward_register(&motor2, M2_FI_GPIO_Port, M2_FI_Pin);
	motor_hard_info_back_register(&motor2, 	  M2_BI_GPIO_Port, M2_BI_Pin);

	motor3.motor_id = MOTOR_MSG_TYPE_3;
	motor_hard_info_forward_register(&motor3, M3_FI_GPIO_Port, M3_FI_Pin);
	motor_hard_info_back_register(&motor3, 	  M3_BI_GPIO_Port, M3_BI_Pin);

	motor4.motor_id = MOTOR_MSG_TYPE_4;
	motor_hard_info_forward_register(&motor4, M4_FI_GPIO_Port, M4_FI_Pin);
	motor_hard_info_back_register(&motor4, 	  M4_BI_GPIO_Port, M4_BI_Pin);

	motor5.motor_id = MOTOR_MSG_TYPE_5;
	motor_hard_info_forward_register(&motor5, M5_FI_GPIO_Port, M5_FI_Pin);
	motor_hard_info_back_register(&motor5, 	  M5_BI_GPIO_Port, M5_BI_Pin);

	motor6.motor_id = MOTOR_MSG_TYPE_6;
	motor_hard_info_forward_register(&motor6, M6_FI_GPIO_Port, M6_FI_Pin);
	motor_hard_info_back_register(&motor6, 	  M6_BI_GPIO_Port, M6_BI_Pin);

	while(1){
		if (rt_mq_recv(&motor_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK){
			motor_ctrl(msg.type, msg.opt);
		}
	}
}

void motor_timer_start(void * parameter)
{
	struct motor_data_t *motor_data = (struct motor_data_t *)parameter;
	switch (motor_data->motor_id){
	case MOTOR_MSG_TYPE_1:
		rt_timer_start(&motor_timer1);
		break;
	case MOTOR_MSG_TYPE_2:
		rt_timer_start(&motor_timer2);
		break;
	case MOTOR_MSG_TYPE_3:
		rt_timer_start(&motor_timer3);
		break;
	case MOTOR_MSG_TYPE_4:
		rt_timer_start(&motor_timer4);
		break;
	case MOTOR_MSG_TYPE_5:
		rt_timer_start(&motor_timer5);
		break;
	case MOTOR_MSG_TYPE_6:
		rt_timer_start(&motor_timer6);
		break;
	default:
		break;
	}
}

void motor1_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_1, MOTOR_DIR_OPEN);
}
void motor2_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_2, MOTOR_DIR_OPEN);
}
void motor3_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_3, MOTOR_DIR_OPEN);
}
void motor4_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_4, MOTOR_DIR_OPEN);
}
void motor5_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_5, MOTOR_DIR_OPEN);
}
void motor6_timer_callback(void *parameter)
{
	motor_send_msg(MOTOR_MSG_TYPE_6, MOTOR_DIR_OPEN);
}

int motor_init(void)
{
	rt_timer_init(&motor_timer1, "motor_timer1", motor1_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	rt_timer_init(&motor_timer2, "motor_timer2", motor2_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	rt_timer_init(&motor_timer3, "motor_timer3", motor3_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	rt_timer_init(&motor_timer4, "motor_timer4", motor4_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	rt_timer_init(&motor_timer5, "motor_timer5", motor5_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	rt_timer_init(&motor_timer6, "motor_timer6", motor6_timer_callback, NULL, MOTOR_PLUS_MS, RT_TIMER_FLAG_ONE_SHOT);
	
	rt_mq_init(&motor_mq,
		"motor_msg",
		&motor_msg_pool[0],
		sizeof(struct motor_msg_t),
		MOTOR_MSG_POOL_SIZE,
		RT_IPC_FLAG_FIFO);

	rt_thread_init(&motor_tid,
			"MOTOR",
			motor_entry,
			RT_NULL,
			&motor_stack[0],
			MOTOR_THREAD_STACK_SIZE,
			MOTOR_THREAD_PRIORITY,
			MOTOR_THREAD_TIMESLICE);
			
	rt_thread_startup(&motor_tid);

	return 0;
}




