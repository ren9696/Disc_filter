/**
 * @file pwr.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef __PWR_H
#define __PWR_H
#include "rtthread.h"

#define ADC_BUF_SIZE 32
#define ADC_BUF_BATTERY_SIZE (ADC_BUF_SIZE / 2)
#define ADC_BUF_PRESSURE_SIZE (ADC_BUF_SIZE / 2)
#define ADC_BATTERY_INDEX 0
#define ADC_PRESSURE_INDEX 1

#define SCREEN_PWR_ON 0
#define SCREEN_PWR_OFF 1 

#define PWR_INTO_SLEEP_TIME (1000 * 30) //ms

struct pressure_data_t {
	float pressure;				//kpa
	rt_uint32_t filtered_voltage_sum;
	
	rt_uint16_t raw_voltage_buf[ADC_BUF_SIZE];
	rt_uint16_t raw_voltage;

	rt_uint16_t filtered_voltage_buf[ADC_BUF_SIZE];
	rt_uint16_t filtered_voltage_index;
	rt_uint16_t filtered_voltage;
	rt_uint8_t bar;		//20 = 2.0bar
};

enum pwr_msg_type_t {
	PWR_MSG_TYPE_NONE = 0,
	PWR_MSG_TYPE_ADC_START,
	PWR_MSG_TYPE_ADC_STOP,
	PWR_MSG_TYPE_GET_BATTERY_VOLTAGE,
	PWR_MSG_TYPE_OFF_SCREEN,
	PWR_MSG_TYPE_ON_SCREEN,
	PWR_MSG_TYPE_WAKE_UP,
	PWR_MSG_TYPE_SLEEP,
};

enum pwr_state_t {
	PWR_STATE_NONE = 0,
	PWR_STATE_SLEEP,
	PWR_STATE_WAKE_UP,
};

struct pwr_data_t
{
        enum pwr_state_t state;
};

struct battery_data_t {
	rt_uint32_t filtered_voltage_sum;

	rt_uint16_t raw_voltage_buf[ADC_BUF_SIZE];
	rt_uint16_t raw_voltage;

	rt_uint16_t filtered_voltage_buf[ADC_BUF_SIZE];
	rt_uint16_t filtered_voltage_index;
	rt_uint16_t filtered_voltage;

	rt_uint16_t voltage;//mv
	rt_uint8_t battery_percent;
};

extern rt_uint16_t adc_buf[];
extern rt_uint16_t adc_value[][2];
extern struct battery_data_t battery_data;
extern struct pressure_data_t pressure_data;
extern struct pwr_data_t pwr_data;

int screen_power_ctrl(int power);
int motor_power_ctrl(int power);
int power_get_battery_voltage(void);
void pwr_msg_send(enum pwr_msg_type_t msg);
void adc_value_handle(void);
int power_intit(void);
#endif
