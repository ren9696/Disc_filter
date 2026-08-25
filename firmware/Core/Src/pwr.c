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
#include "screen.h"
rt_uint16_t adc_value[ADC_BUF_SIZE][2];
struct battery_data_t battery_data;
struct pressure_data_t pressure_data;
struct pwr_data_t pwr_data;
static struct rt_thread pwr_tid;
#define PWR_BATTERY_FULL_MV 4100
#define PWR_THREAD_STACK_SIZE 256
#define PWR_THREAD_PRIORITY 14
#define PWR_THREAD_TIMESLICE 20
#define PWR_MSG_POOL_SIZE 16
static char pwr_stack[PWR_THREAD_STACK_SIZE];
static struct rt_messagequeue pwr_mq;
static rt_uint8_t pwr_msg_pool[PWR_MSG_POOL_SIZE];
INIT_APP_EXPORT(power_intit);
struct rt_timer pwr_timer;

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

/**
 * @brief 电机电源控制
 * 
 * @param power 0: 关闭电机电源
 *              1: 打开电机电源
 * @return int 
 */
int motor_power_ctrl(int power)
{
	if (power == 0){
		HAL_GPIO_WritePin(M_PWR_EN_GPIO_Port, 
				  M_PWR_EN_Pin, 
				  GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(M_PWR_EN_GPIO_Port, 
				  M_PWR_EN_Pin, 
				  GPIO_PIN_SET);
	}

	return 0;
}

void pwr_msg_send(enum pwr_msg_type_t msg)
{
	rt_mq_send(&pwr_mq, &msg, sizeof(enum pwr_msg_type_t));
}

void adc_voltage_filter()
{
	battery_data.filtered_voltage_sum -= battery_data.filtered_voltage_buf[battery_data.filtered_voltage_index];
        battery_data.filtered_voltage_buf[battery_data.filtered_voltage_index] = battery_data.raw_voltage;
	battery_data.filtered_voltage_sum += battery_data.filtered_voltage_buf[battery_data.filtered_voltage_index];
	battery_data.filtered_voltage_index++;
	if (battery_data.filtered_voltage_index >= ADC_BUF_SIZE){
		battery_data.filtered_voltage_index = 0;
	}
	battery_data.filtered_voltage = battery_data.filtered_voltage_sum / ADC_BUF_SIZE;

	pressure_data.filtered_voltage_sum -= pressure_data.filtered_voltage_buf[pressure_data.filtered_voltage_index];
        pressure_data.filtered_voltage_buf[pressure_data.filtered_voltage_index] = pressure_data.raw_voltage;
	pressure_data.filtered_voltage_sum += pressure_data.filtered_voltage_buf[pressure_data.filtered_voltage_index];
	pressure_data.filtered_voltage_index++;
	if (pressure_data.filtered_voltage_index >= ADC_BUF_SIZE){
		pressure_data.filtered_voltage_index = 0;
	}
	pressure_data.filtered_voltage = pressure_data.filtered_voltage_sum / ADC_BUF_SIZE;
}

void adc_average_voltage_get()
{
	rt_uint32_t battery_sum = 0;
	rt_uint32_t presure_sum = 0;
	for (int i = 0; i < ADC_BUF_SIZE; i++){
		battery_data.raw_voltage_buf[i] = adc_value[i][ADC_BATTERY_INDEX] * 0.8056640625;
		pressure_data.raw_voltage_buf[i] = adc_value[i][ADC_PRESSURE_INDEX] * 0.8056640625;
		battery_sum += battery_data.raw_voltage_buf[i];
		presure_sum += pressure_data.raw_voltage_buf[i];
		battery_data.raw_voltage = battery_sum / ADC_BUF_SIZE;
		pressure_data.raw_voltage = presure_sum / ADC_BUF_SIZE;
	}
}

void battery_percent_get(struct battery_data_t *battery_data)
{
        battery_data->voltage = battery_data->filtered_voltage * 2;
        float temp_f = (float)battery_data->voltage / PWR_BATTERY_FULL_MV;
        battery_data->battery_percent = temp_f * 100;
        if (battery_data->battery_percent > 100){
                battery_data->battery_percent  = 100;
        } else if (battery_data->battery_percent <= 0){
                battery_data->battery_percent = 0;
        }
}

void pressure_get(struct pressure_data_t *pressure_data)
{
	float tempV = (float)pressure_data->filtered_voltage / 1000;
	tempV *= 2;
	if (tempV < 0.2){
		pressure_data->pressure = 0;
		pressure_data->bar = 0;
	} else {
		pressure_data->pressure = (tempV - 0.2) / 0.006429;
		pressure_data->bar = pressure_data->pressure / 10;
	}
}

void adc_value_handle()
{
	adc_average_voltage_get();
	adc_voltage_filter();
	battery_percent_get(&battery_data);
	pressure_get(&pressure_data);
}

void pwr_wake_up()
{
        if (pwr_data.state == PWR_STATE_SLEEP){
		pwr_data.state = PWR_STATE_WAKE_UP;
		ht1621b_init();
	}

	rt_timer_stop(&pwr_timer);
	rt_timer_start(&pwr_timer);
}

void pwr_sleep()
{
        pwr_data.state = PWR_STATE_SLEEP;
	ht1621b_close();
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void pwr_entry(void *parameter)
{
	enum pwr_msg_type_t msg;

	while (1){
		rt_mq_recv(&pwr_mq, &msg, sizeof(enum pwr_msg_type_t), RT_WAITING_FOREVER);
		switch (msg){
			case PWR_MSG_TYPE_GET_BATTERY_VOLTAGE:
				break;
			
			case PWR_MSG_TYPE_OFF_SCREEN:
				screen_power_ctrl(SCREEN_PWR_OFF);
				break;

			case PWR_MSG_TYPE_ON_SCREEN:
				screen_power_ctrl(SCREEN_PWR_ON);
				break;

			case PWR_MSG_TYPE_WAKE_UP:
				pwr_wake_up();
				break;

			case PWR_MSG_TYPE_SLEEP:
				pwr_sleep();
				break;
		}
	}
}

void pwr_timer_callback(void * arg)
{
        pwr_msg_send(PWR_MSG_TYPE_SLEEP);
}

int power_intit()
{
	rt_mq_init(&pwr_mq,
			"pwr_msg",
			&pwr_msg_pool[0],             
			sizeof(enum pwr_msg_type_t),
			PWR_MSG_POOL_SIZE,        
			RT_IPC_FLAG_FIFO);      

	rt_thread_init(&pwr_tid,
			"pwr",
			pwr_entry,
			RT_NULL,
			pwr_stack,
			PWR_THREAD_STACK_SIZE,
			PWR_THREAD_PRIORITY, 
			PWR_THREAD_TIMESLICE);
	rt_thread_startup(&pwr_tid);

	rt_timer_init(&pwr_timer, "pwr_timer",
			pwr_timer_callback,
			NULL, 
			PWR_INTO_SLEEP_TIME, 
			RT_TIMER_FLAG_ONE_SHOT); 
	rt_timer_start(&pwr_timer);	
	return 0;
}

/**
 * @brief ADC转换完成回调函数,暂时无处理
 * 
 * @param hadc 
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1){
	}
}

/**
 * @brief 启动ADC转换，暂时无调用
 * 
 * @return int 
 */
int power_get_battery_voltage()
{
	HAL_ADC_Start_DMA(&hadc1, (rt_uint32_t*)adc_value, ADC_BUF_SIZE);
	return 0;
}
