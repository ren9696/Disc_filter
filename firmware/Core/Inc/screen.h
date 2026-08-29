/**
 * @file screen.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SCREEN_H
#define SCREEN_H
#include "stdint.h"
#define HT1621B_CS_LOW()  	HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_RESET)
#define HT1621B_CS_HIGH()	HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_SET)
#define HT1621B_WR_LOW()	HAL_GPIO_WritePin(SCREEN_WR_GPIO_Port, SCREEN_WR_Pin, GPIO_PIN_RESET)
#define HT1621B_WR_HIGH()	HAL_GPIO_WritePin(SCREEN_WR_GPIO_Port, SCREEN_WR_Pin, GPIO_PIN_SET)
#define HT1621B_DATA_LOW()	HAL_GPIO_WritePin(SCREEN_DATA_GPIO_Port, SCREEN_DATA_Pin, GPIO_PIN_RESET)
#define HT1621B_DATA_HIGH()	HAL_GPIO_WritePin(SCREEN_DATA_GPIO_Port, SCREEN_DATA_Pin, GPIO_PIN_SET)

#define HT1621_CMD_SYS_DIS      (0x00)         
#define HT1621_CMD_SYS_EN       (0x02) 
#define HT1621_CMD_RC_OSC       (0x30)
#define HT1621_CMD_OFF          (0x02)          
#define HT1621_CMD_ON           (0x07)          
#define HT1621_CMD_WRITE        (0xA0)          
#define HT1621_CMD_BIAS_COM 	(0x52)

#define SEG_A   (1 << 0)
#define SEG_B   (1 << 1)
#define SEG_C   (1 << 2)
#define SEG_D   (1 << 3)
#define SEG_E   (1 << 4)
#define SEG_F   (1 << 5)
#define SEG_G   (1 << 6)
#define SEG_DP  (1 << 7)

enum screen_symobol_e {
	SCREEN_SYMOBOL_NONE=0,
	SCREEN_SYMOBOL_M1,
	SCREEN_SYMOBOL_M2,
	SCREEN_SYMOBOL_M3,
	SCREEN_SYMOBOL_M4,
	SCREEN_SYMOBOL_M5,
	SCREEN_SYMOBOL_M6,
	SCREEN_SYMOBOL_M7,
	SCREEN_SYMOBOL_M8,
	SCREEN_SYMOBOL_M9,
	SCREEN_SYMOBOL_M10,
	SCREEN_SYMOBOL_M11,
	SCREEN_SYMOBOL_M12,
	SCREEN_SYMOBOL_M13,
	SCREEN_SYMOBOL_M14,
	SCREEN_SYMOBOL_M15,
	SCREEN_SYMOBOL_M16,
	SCREEN_SYMOBOL_M17,
	SCREEN_SYMOBOL_M18,
	SCREEN_SYMOBOL_M19,
	SCREEN_SYMOBOL_X0,
	SCREEN_SYMOBOL_X1,
	SCREEN_SYMOBOL_X2
};

enum screen_main_page_e {
	SCREEN_PAGE_REALTIME = 0,
	SCREEN_PAGE_SETVIEWER,
	SCREEN_PAGE_SETING,
	SCREEN_PAGE_REVERSING,
	SCREEN_PAGE_STATION_INTERVAL,
	SCREEN_PAGE_4,
};

enum screen_realtime_page_e {
	REALTIME_PAGE_BAR_DIFF = 0,
	REALTIME_PAGE_REVERSE_PERIOD,
	SCREEN_REALTIME_PAGE_CNT
};

enum screen_setviewer_page_e {
	SETVIEWER_REAL_BAR_DIFF = 0,   
	SETVIEWER_REVER_TIME_BAR_DIFF,
	SETVIEWER_REVER_TIME_PERIOD,
	SETVIEWER_KNOW,
	SETVIEWER_PAGE_CNT
};

enum screen_setting_page_e {
	SETTING_PAGE_NONE = 0,
	SETING_PAGE_REVER_PERIOD,
	SETING_PAGE_REVER_TIME,
	SETING_PAGE_STATION_INTERVAL,
	SETING_PAGE_REVER_BAR_DIFF,
	SETING_PAGE_MAIN_VALVE,
	SETING_PAGE_STATION_NUM,
	SETING_PAGE_CNT
};

struct screen_data_t {
	enum screen_main_page_e main_page_index;
	enum screen_realtime_page_e realtime_page_index;
	enum screen_setviewer_page_e setviewer_page_index;
	enum screen_setting_page_e setting_page_index;
};

extern struct screen_data_t screen_data;

int screen_init(void);
void ht1621_display_symbol(uint8_t t_index, uint8_t on);
void ht1621_display_p(uint8_t p_num, uint8_t on);
void ht1621_display_number(uint8_t area, uint8_t num);
void ht1621_set_pixel(uint8_t seg, uint8_t com, uint8_t on);
static void screen_display_state_real_bar_diff(void);
void screen_main_page_switch(enum screen_main_page_e page);
void screen_setviewer_page_switch(void);
void screen_seting_page_switch(void );
static void screen_display_setting_rever_period(void);
void ht1621b_close(void);
void ht1621b_init(void);
#endif
