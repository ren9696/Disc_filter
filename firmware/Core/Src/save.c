/**
 * @file save.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "save.h"
#include "main.h"
#include "rtthread.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
#define FLASH_USER_START_ADDR   	((rt_uint32_t)0x0803F800)
#define FLASH_USER_PAGE_NUM 		((rt_uint32_t)127)
#define FLASH_USER_WORD_SIZE		((rt_uint32_t)4)
struct save_set_data_t save_set_data;

#if 1
#define SAVE_THREAD_PRIORITY 	11
#define SAVE_THREAD_TIMESLICE 	20
#define SAVE_THREAD_STACK_SIZE 	256
static struct rt_thread save_tid;
static char save_stack[SAVE_THREAD_STACK_SIZE];
INIT_APP_EXPORT(save_init);
static struct rt_messagequeue save_mq;
#define SAVE_MSG_POOL_SIZE 16
static rt_uint8_t save_msg_pool[SAVE_MSG_POOL_SIZE];
#endif


/**
 * @brief 向flash中写入数据
 * 
 * @return int 
 */
int save_write_data_to_flash(void)
{
        rt_uint32_t PAGEError;
	rt_uint32_t addr_cnt = 0;

  	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
	EraseInitStruct.NbPages     = 1;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++, 
				save_set_data.rever_period_h) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.rever_period_m) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.rever_time_m) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.rever_time_s) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.rever_bar_diff) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.station_interval) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.station_num) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.main_valve) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
				FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++,
				save_set_data.save_flash_default_id) != HAL_OK){
		HAL_FLASH_Lock();
		return -1;
	}

	HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief 向flash中写入默认数据
 * 
 * @return int 0:成功 -1:失败
 */
int write_all_default_data_to_flash(void)
{
	save_set_data.rever_period_h 			= SAVE_FLASH_DEFAULT_REVER_PERIOD_H;
	save_set_data.rever_period_m 			= SAVE_FLASH_DEFAULT_REVER_PERIOD_M;
	save_set_data.rever_time_m 			= SAVE_FLASH_DEFAULT_REVER_TIME_M;
	save_set_data.rever_time_s 			= SAVE_FLASH_DEFAULT_REVER_TIME_S;
	save_set_data.rever_bar_diff 			= SAVE_FLASH_DEFAULT_REVER_BAR_DIFF;
	save_set_data.station_interval 			= SAVE_FLASH_DEFAULT_STATION_INTERVAL;
	save_set_data.station_num 			= SAVE_FLASH_DEFAULT_STATION_NUM;
	save_set_data.main_valve 			= SAVE_FLASH_DEFAULT_MAIN_VALVE;
	save_set_data.save_flash_default_id 		= SAVE_FLASH_DEFAULT_ID;

	if (save_write_data_to_flash() != 0){
		return -1;
	}

	return 0;
}

/**
 * @brief 从flash中读取所有数据
 * 
 */
void save_read_all_data_from_flash(void)
{
	rt_uint32_t addr_cnt = 0;
	HAL_FLASH_Unlock();
	
	save_set_data.rever_period_h 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.rever_period_m 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.rever_time_m 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.rever_time_s 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.rever_bar_diff 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.station_interval 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.station_num 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.main_valve 		= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);
	save_set_data.save_flash_default_id 	= *(rt_uint32_t *)(FLASH_USER_START_ADDR + FLASH_USER_WORD_SIZE * addr_cnt++);

	save_set_data.rever_period_sum_s = save_set_data.rever_period_h * 3600 + save_set_data.rever_period_m * 60;
	save_set_data.rever_time_sum_s = save_set_data.rever_time_m * 60 + save_set_data.rever_time_s;
	HAL_FLASH_Lock();
}


int save_send_msg(enum save_msg_type_t msg)
{
	enum save_msg_type_t send_msg;
	send_msg = msg;
	
	if (rt_mq_send(&save_mq, &send_msg, sizeof(enum save_msg_type_t)) != RT_EOK){
		return -1;
	}

	return 0;
}

void save_entry(void *parameter)
{
	#if 1
	save_read_all_data_from_flash();
	if (save_set_data.save_flash_default_id != SAVE_FLASH_DEFAULT_ID){
		write_all_default_data_to_flash();
	}
	#endif

	enum save_msg_type_t msg;

	while(1){
		if (rt_mq_recv(&save_mq, &msg, sizeof(enum save_msg_type_t), RT_WAITING_FOREVER) == RT_EOK){
			switch(msg){
				case SAVE_MSG_TYPE_SAVE_ALL_DATA:
					save_write_data_to_flash();
					break;
				default:
					break;
			}
		}
	}
}

int save_init(void)
{	
	rt_mq_init(&save_mq,
			"save_msg",
			&save_msg_pool[0],             
			sizeof(enum save_msg_type_t),
			SAVE_MSG_POOL_SIZE,        
			RT_IPC_FLAG_FIFO);       

	rt_thread_init(&save_tid,
			"SAVE",
			save_entry,
			RT_NULL,
			&save_stack[0],
			SAVE_THREAD_STACK_SIZE,
			SAVE_THREAD_PRIORITY,
			SAVE_THREAD_TIMESLICE);
			
	rt_thread_startup(&save_tid);

	return 0;
}


