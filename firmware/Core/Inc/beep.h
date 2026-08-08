/**
 * @file beep.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef __BEEP_H
#define __BEEP_H
#include "stdint.h"

enum beep_smg_type_e{
	BEEP_MSG_TYPE_NONE=0,
	BEEP_MSG_TYPE_1,
};

int beep_init(void);
void beep_send(enum beep_smg_type_e type);
#endif
