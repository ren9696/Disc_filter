/**
 * @file motor.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MOTOR_H
#define MOTOR_H
#include "main.h"

#define MOTOR_PIN_STATE_LOW  0
#define MOTOR_PIN_STATE_HIGH 1

enum motor_msg_type_t {
	MOTOR_MSG_TYPE_NONE = 0,
	MOTOR_MSG_TYPE_1,
	MOTOR_MSG_TYPE_2,
	MOTOR_MSG_TYPE_3,
	MOTOR_MSG_TYPE_4,
	MOTOR_MSG_TYPE_5,
	MOTOR_MSG_TYPE_6
};

enum motor_opt_t {
	MOTOR_DIR_NONE = 0,
	MOTOR_DIR_FORWARD,
	MOTOR_DIR_BACKWARD,
	MOTOR_DIR_STOP,
	MOTOR_DIR_OPEN
};

struct motor_msg_t {
	enum motor_msg_type_t type;
    	enum motor_opt_t opt;
};

struct motor_hard_info_t {
	GPIO_TypeDef *port;
	uint16_t pin;
};

struct motor_data_t {
       struct motor_hard_info_t hard_info_forward;
       struct motor_hard_info_t hard_info_back;
       enum motor_opt_t motor_state;
       enum motor_msg_type_t motor_id;
};

int motor_send_msg(enum motor_msg_type_t msg_type, enum motor_opt_t dir);
int motor_init(void);
void motor_timer_start(void * parameter);
#endif
