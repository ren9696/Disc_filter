/**
 * @file screen.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
 
#include "screen.h"
#include "main.h"
#include "rtthread.h"
#include "pwr.h"
#include "save.h"
#include "work.h"

static struct rt_thread screen_tid;
struct rt_timer screen_work_timer;
#define SCREEN_THREAD_STACK_SIZE 1024
#define SCREEN_THREAD_PRIORITY 16
#define SCREEN_THREAD_TIMESLICE 20
static char screen_stack[SCREEN_THREAD_STACK_SIZE];
INIT_APP_EXPORT(screen_init);
#define HT1621B_CS_LOW()  	HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_RESET)
#define HT1621B_CS_HIGH()	HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_SET)
#define HT1621B_WR_LOW()	HAL_GPIO_WritePin(SCREEN_WR_GPIO_Port, SCREEN_WR_Pin, GPIO_PIN_RESET)
#define HT1621B_WR_HIGH()	HAL_GPIO_WritePin(SCREEN_WR_GPIO_Port, SCREEN_WR_Pin, GPIO_PIN_SET)
#define HT1621B_DATA_LOW()	HAL_GPIO_WritePin(SCREEN_DATA_GPIO_Port, SCREEN_DATA_Pin, GPIO_PIN_RESET)
#define HT1621B_DATA_HIGH()	HAL_GPIO_WritePin(SCREEN_DATA_GPIO_Port, SCREEN_DATA_Pin, GPIO_PIN_SET)
#define SCREEN_SYMOBOL_CNT 19+1+3  // 3=x123
#define SCREEN_RAM_BUFF_SIZE (32)
rt_uint32_t screen_realtime_page_switch_cnt = 0;
uint8_t screen_display_ram[SCREEN_RAM_BUFF_SIZE]; 
struct screen_data_t screen_data;
#define SCREEN_DISPLAY_PERIOD 100 //MS
#define SCREEN_DISPLAY_REALPAGE_SWITCH_TIME 4000 //MS
static uint8_t screen_symobol_list[SCREEN_SYMOBOL_CNT] = { //seg com
	0,
	(1  << 4) | 1, 
	(1  << 4) | 2,  
	(1  << 4) | 3,  
	(1  << 4) | 4,  
	(5  << 4) | 4,  
	(5  << 4) | 3,  
	(5  << 4) | 2,  
	(5  << 4) | 1,  
	(2  << 4) | 2,  
	(2  << 4) | 1,  
	(3  << 4) | 4,  
	(6  << 4) | 4,  
	(11 << 4) | 4,  
	(15 << 4) | 3,  
	(10 << 4) | 4,  
	(15 << 4) | 2,  
	(10 << 4) | 2,  
	(10 << 4) | 1,  
	(15 << 4) | 1,
	
	(2  << 4) | 3,
	(2  << 4) | 4,
	(15 << 4) | 4,
};

const uint8_t number_table[] = {
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // 0       
	SEG_B | SEG_C,                                        
	SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,                   
	SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,                   
	SEG_F | SEG_G | SEG_B | SEG_C,                           
	SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,                   
	SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D,           
	SEG_A | SEG_B | SEG_C,                                   
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,           // 9
	0,					// 10 全灭
};

static uint8_t seg_area[][7] = {
	{0},
	{
		(7  << 4) | 4,  // 1A   SEG7 COM4
		(7  << 4) | 3,  // 1B   SEG7 COM3
		(7  << 4) | 2,  // 1C   SEG7 COM2
		(7  << 4) | 1,  // 1D   SEG7 COM1
		(6  << 4) | 1,  // 1E   SEG6 COM1
		(6  << 4) | 3,  // 1F   SEG6 COM3
		(6  << 4) | 2,  // 1G   SEG6 COM2
	},
	{
		(9  << 4) | 4,  // 2A   SEG9 COM4
		(9  << 4) | 3,  // 2B   SEG9 COM3
		(9  << 4) | 2,  // 2C   SEG9 COM2
		(9  << 4) | 1,  // 2D   SEG9 COM1
		(8  << 4) | 1,  // 2E   SEG8 COM1
		(8  << 4) | 3,  // 2F   SEG8 COM3
		(8  << 4) | 2,  // 2G   SEG8 COM2
	},
	{
		(12 << 4) | 4,  // 3A   SEG12 COM4
		(12 << 4) | 3,  // 3B   SEG12 COM3
		(12 << 4) | 2,  // 3C   SEG12 COM2
		(12 << 4) | 1,  // 3D   SEG12 COM1
		(11 << 4) | 1,  // 3E   SEG11 COM1
		(11 << 4) | 3,  // 3F   SEG11 COM3
		(11 << 4) | 2,  // 3G   SEG11 COM2
	},
	{
		(14 << 4) | 4,  // 4A   SEG14 COM4
		(14 << 4) | 3,  // 4B   SEG14 COM3
		(14 << 4) | 2,  // 4C   SEG14 COM2
		(14 << 4) | 1,  // 4D   SEG14 COM1
		(13 << 4) | 1,  // 4E   SEG13 COM1
		(13 << 4) | 3,  // 4F   SEG13 COM3
		(13 << 4) | 2,  // 4G   SEG13 COM2
	},
	{
		(4  << 4) | 4,  // 5A   SEG4 COM4
		(4  << 4) | 3,  // 5B   SEG4 COM3
		(4  << 4) | 2,  // 5C   SEG4 COM2
		(4  << 4) | 1,  // 5D   SEG4 COM1
		(3  << 4) | 1,  // 5E   SEG3 COM1
		(3  << 4) | 3,  // 5F   SEG3 COM3
		(3  << 4) | 2,  // 5G   SEG3 COM2
	}
};

void rt_hw_us_delay(rt_uint32_t us)
{
	#if 0
	    rt_uint64_t ticks;
	    rt_uint32_t told, tnow, tcnt = 0;
	    rt_uint32_t reload = SysTick->LOAD;

	    ticks = us * (reload / (1000000 / RT_TICK_PER_SECOND));
	    told = SysTick->VAL;
	    while (1)
	    {
		tnow = SysTick->VAL;
		if (tnow != told)
		{
		    if (tnow < told)
		    {
			tcnt += told - tnow;
		    }
		    else
		    {
			tcnt += reload - tnow + told;
		    }
		    told = tnow;
		    if (tcnt >= ticks)
		    {
			break;
		    }
		}
	    }
	#endif
}

/**
 * @brief 反冲洗次数
 * 
 * @param num 
 */
static void screen_display_reverse_num(int num)
{
	int num1 = (num / 1000) % 10;
	int num2 = (num / 100) % 10;
	int num3 = (num / 10) % 10;
	int num4 = num % 10;
	
	ht1621_display_number(1, num1);
	ht1621_display_number(2, num2);
	ht1621_display_number(3, num3);
	ht1621_display_number(4, num4);
}

static void screen_display_real_bardiff_num(uint8_t num)
{
	if (num > 99){
		num = 99;
	}
	uint8_t num1 = num / 10;
	uint8_t num2 = num % 10;
        ht1621_display_number(1, num1);
	ht1621_display_number(2, num2);
}

void ht1621_display_p(uint8_t p_num, uint8_t on)
{
	uint8_t segp1, comp1, segp2, comp2;
	segp1 = 8-1;
	comp1 = 4-1;
	segp2 = 13-1;
	comp2 = 4-1;
	if (p_num == 1){
		ht1621_set_pixel(segp1, comp1, on);
	} else if (p_num == 2){
		ht1621_set_pixel(segp2, comp2, on);
	}
}


static void screen_display_time_symbol()
{
	uint8_t s, c;
	s = 10-1;
	c = 3-1;
	ht1621_set_pixel(s, c, 1);
}

static void screen_display_state_reverse_Period()
{
        ht1621_display_symbol(SCREEN_SYMOBOL_M1, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M15, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M14, 1);
	screen_display_time_symbol();

	uint8_t h1 = save_set_data.rever_period_h / 10;
	uint8_t h2 = save_set_data.rever_period_h % 10;
	uint8_t m1 = save_set_data.rever_period_m / 10;
	uint8_t m2 = save_set_data.rever_period_m % 10;
	
        ht1621_display_number(1, h1);
	ht1621_display_number(2, h2);
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void ht1621_write_bit(uint8_t data)
{
	HT1621B_WR_LOW();
	rt_hw_us_delay(1);
	
	if(data){
		HT1621B_DATA_HIGH();
	} else {
		HT1621B_DATA_LOW();
	}

	HT1621B_WR_HIGH();
	rt_hw_us_delay(1);
}


void ht1621_write_cmd(uint8_t cmd)
{
	uint8_t data = cmd;
        HT1621B_CS_LOW();
	rt_hw_us_delay(1);

	ht1621_write_bit(1);
	ht1621_write_bit(0);
	ht1621_write_bit(0);

	ht1621_write_bit(0);	
	for (int i = 0; i < 8; i++){
		ht1621_write_bit(data & 0x80);
		data <<= 1;
	}

	rt_hw_us_delay(1);
	HT1621B_CS_HIGH();
	rt_hw_us_delay(1);
}

void ht1621_write_bit_h(uint8_t data, uint8_t len)
{
	for (int i = 0; i < len; i++){	
		HT1621B_WR_LOW();
		rt_hw_us_delay(1);
		
		if (data & 0x80){
			HT1621B_DATA_HIGH();
		} else {
			HT1621B_DATA_LOW();
		}
		rt_hw_us_delay(1);
		
		HT1621B_WR_HIGH();
		rt_hw_us_delay(1);
		
		data <<= 1;
	}
}

void ht1621_write_bit_l(uint8_t data, uint8_t len)
{
	for (int i = 0; i < len; i++){		
		HT1621B_WR_LOW();
		rt_hw_us_delay(1);
		
		if (data & 0x01){
			HT1621B_DATA_HIGH();
		} else {
			HT1621B_DATA_LOW();
		}

		rt_hw_us_delay(1);
		HT1621B_WR_HIGH();
		rt_hw_us_delay(1);
		data >>= 1;
	}
}

void ht1621b_write_data(uint8_t seg, uint8_t data)
{	
	HT1621B_CS_LOW();
	rt_hw_us_delay(1);

	ht1621_write_bit_h(0xa0, 3);
	rt_hw_us_delay(1);

	ht1621_write_bit_h(seg<<2, 6);
	rt_hw_us_delay(1);

	ht1621_write_bit_l(data, 4);
	rt_hw_us_delay(1);

	HT1621B_CS_HIGH();
	rt_hw_us_delay(1);
	HT1621B_WR_HIGH();
	rt_hw_us_delay(1);
}

void ht1621_set_pixel(uint8_t seg, uint8_t com, uint8_t on)
{
    	uint8_t idx = seg ; 
    	uint8_t bit_pos = com;

	if (on){
		screen_display_ram[idx] |= (1 << bit_pos);
	} else{
		screen_display_ram[idx] &= ~(1 << bit_pos);
	}
}

void ht1621_display_symbol(uint8_t t_index, uint8_t on)
{
	static uint8_t s;
	static uint8_t c;
	if (t_index < SCREEN_SYMOBOL_CNT){
        	uint8_t map_val = screen_symobol_list[t_index];
		if (map_val != 0xFF){
			s = (map_val >> 4) & 0x0F;
			c = map_val & 0x0F;
			s-=1;				// 1621的SEG0硬件连接到了屏幕的SEG1,所以这里要减1
			c-=1;				
			ht1621_set_pixel(s, c, on);
		}
	}
}

void ht1621_updata(void)
{
	for (int i = 0; i < SCREEN_RAM_BUFF_SIZE; i++){
		ht1621b_write_data(i, screen_display_ram[i]);
	}
}

void ht1621_clear(void)
{
    	rt_memset(screen_display_ram, 0x00, sizeof(screen_display_ram));
    	//ht1621_updata();
}

void screen_page_none(void)
{
	ht1621_clear();
}

void ht1621_display_number(uint8_t area, uint8_t num)
{
	uint8_t dis_num = number_table[num];
	uint8_t seg, com;
	for (int i = 0; i < 7; i++){
	        if (dis_num & (1 << i)){
			uint8_t s_c = seg_area[area][i];
			seg = s_c >> 4 & 0x0F;
			com = s_c & 0x0F;
			seg-=1;
			com-=1;
			ht1621_set_pixel(seg, com, 1);
		} else {
			uint8_t s_c = seg_area[area][i];
			seg = s_c >> 4  & 0x0F;
			com = s_c & 0x0F;
			seg-=1;
			com-=1;
			ht1621_set_pixel(seg, com, 0);
		}
	}
}

void ht1621b_close(void)
{
	screen_power_ctrl(SCREEN_PWR_OFF);
	rt_thread_mdelay(100);
	HT1621B_CS_HIGH();
	HT1621B_WR_HIGH();
	HT1621B_DATA_HIGH();
	rt_thread_delay(10);
	ht1621_write_cmd(HT1621_CMD_SYS_DIS);
	ht1621_write_cmd(HT1621_CMD_OFF);
	rt_thread_mdelay(5);
	
}

void ht1621b_init(void)
{
	screen_power_ctrl(SCREEN_PWR_ON);
	rt_thread_mdelay(100);
	HT1621B_CS_HIGH();
	HT1621B_WR_HIGH();
	HT1621B_DATA_HIGH();
	rt_thread_delay(10);
	ht1621_write_cmd(HT1621_CMD_SYS_EN);
	ht1621_write_cmd(HT1621_CMD_ON);
	ht1621_write_cmd(HT1621_CMD_RC_OSC);
	ht1621_write_cmd(HT1621_CMD_BIAS_COM);
	rt_thread_mdelay(5);
}

static void screen_display_battery()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_X2, 1);
	if (battery_data.battery_percent >= 50){
		ht1621_display_symbol(SCREEN_SYMOBOL_X0, 1);
		ht1621_display_symbol(SCREEN_SYMOBOL_X1, 1);
	} else if (battery_data.battery_percent < 10){
		ht1621_display_symbol(SCREEN_SYMOBOL_X0, 0);
		ht1621_display_symbol(SCREEN_SYMOBOL_X1, 0);
	} else if (battery_data.battery_percent < 50){
		ht1621_display_symbol(SCREEN_SYMOBOL_X0, 1);
		ht1621_display_symbol(SCREEN_SYMOBOL_X1, 0);
	}
}


static void screen_display_state_real_bar_diff(void)
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M12, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M13, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M18, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M19, 1);
	ht1621_display_p(1, 1);
	ht1621_display_p(2, 1);
	screen_display_real_bardiff_num(pressure_data.bar);
	screen_display_reverse_num(save_set_data.rever_bar_diff);
}

static void screen_display_realtime()
{
	screen_realtime_page_switch_cnt++;
	if (screen_realtime_page_switch_cnt > SCREEN_DISPLAY_REALPAGE_SWITCH_TIME/SCREEN_DISPLAY_PERIOD){ 
		screen_realtime_page_switch_cnt = 0;
		ht1621_clear();
		screen_data.realtime_page_index++;
		if (screen_data.realtime_page_index >= SCREEN_REALTIME_PAGE_CNT){
			screen_data.realtime_page_index = REALTIME_PAGE_BAR_DIFF;
		}
	}

	switch (screen_data.realtime_page_index){
		case REALTIME_PAGE_BAR_DIFF:
			screen_display_state_real_bar_diff();
			break;

		case REALTIME_PAGE_REVERSE_PERIOD:
			screen_display_state_reverse_Period();
			break;

		default:
			break;
	}
}

static void screen_setviewer_real_bar_diff()
{
        ht1621_display_symbol(SCREEN_SYMOBOL_M12, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M18, 1);
	ht1621_display_p(1, 1);
	screen_display_real_bardiff_num(pressure_data.bar);

}

static void screen_setviewer_rever_time_bar_diff()
{
        ht1621_display_symbol(SCREEN_SYMOBOL_M6, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M8, 1);
	screen_display_real_bardiff_num(0);
	screen_display_reverse_num(0);
}

static void screen_setviewer_rever_time_period()
{
        ht1621_display_symbol(SCREEN_SYMOBOL_M6, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M7, 1);
	screen_display_real_bardiff_num(0);
	screen_display_reverse_num(process_data.reverse_period_num);//反洗周期次数
}

static void screen_setviewer_know()
{
	screen_display_real_bardiff_num(0);
	screen_display_reverse_num(0);
}

static void screen_display_main_setviewer()
{
	switch (screen_data.setviewer_page_index){
		case SETVIEWER_REAL_BAR_DIFF:
			screen_setviewer_real_bar_diff();
			break;

		case SETVIEWER_REVER_TIME_BAR_DIFF:
			screen_setviewer_rever_time_bar_diff();
			break;

		case SETVIEWER_REVER_TIME_PERIOD:
			screen_setviewer_rever_time_period();
			break;
			
		case SETVIEWER_KNOW:
			screen_setviewer_know();
			break;

		default:
			break;
	}
}

static void screen_display_setting_rever_period()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M1, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M15, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M14, 1);
	screen_display_time_symbol();
	
	uint8_t h1 = save_set_data.rever_period_h / 10;
	uint8_t h2 = save_set_data.rever_period_h % 10;
	uint8_t m1 = save_set_data.rever_period_m / 10;
	uint8_t m2 = save_set_data.rever_period_m % 10;
	
	ht1621_display_number(1, h1);
	ht1621_display_number(2, h2);
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void screen_display_setting_rever_time()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M2, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M17, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M16, 1);
	uint8_t h1 = save_set_data.rever_time_m / 10;
	uint8_t h2 = save_set_data.rever_time_m % 10;
	uint8_t m1 = save_set_data.rever_time_s / 10;
	uint8_t m2 = save_set_data.rever_time_s % 10;
	ht1621_display_number(1, h1);
	ht1621_display_number(2, h2);
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void screen_display_setting_station_interval()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M3, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M16, 1);
	uint8_t m1 = save_set_data.station_interval / 10;
	uint8_t m2 = save_set_data.station_interval % 10;
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void screen_display_setting_rever_bar_diff()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M4, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M13, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M19, 1);
	ht1621_display_p(2, 1);
	uint8_t m1 = save_set_data.rever_bar_diff / 10;
	uint8_t m2 = save_set_data.rever_bar_diff % 10;
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void screen_display_setting_main_valve()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M11, 1);
	uint8_t m3 = save_set_data.main_valve / 10;
	uint8_t m4 = save_set_data.main_valve % 10;
	ht1621_display_number(3, m3);
	ht1621_display_number(4, m4);
}

static void screen_display_setting_station_num()
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M5, 1);
	uint8_t m1 = save_set_data.station_num / 10;
	uint8_t m2 = save_set_data.station_num % 10;
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

static void screen_display_main_setting()
{
	switch (screen_data.setting_page_index){
		case SETING_PAGE_REVER_PERIOD:
			screen_display_setting_rever_period();
			break;

		case SETING_PAGE_REVER_TIME:
			screen_display_setting_rever_time();
			break;

		case SETING_PAGE_STATION_INTERVAL:
			screen_display_setting_station_interval();
			break;

		case SETING_PAGE_REVER_BAR_DIFF:
			screen_display_setting_rever_bar_diff();
			break;

		case SETING_PAGE_MAIN_VALVE:
			screen_display_setting_main_valve();
			break;

		case SETING_PAGE_STATION_NUM:
			screen_display_setting_station_num();
			break;
			
		default:
			break;
	}
}

void screen_seting_page_switch(void )
{
	ht1621_clear();
	screen_data.setting_page_index++;
	if (screen_data.setting_page_index >= SETING_PAGE_CNT){
		screen_data.setting_page_index = SETING_PAGE_REVER_PERIOD;
	}
}

void screen_display_main_reversing(void)
{
	uint8_t minute = 0;
	uint8_t second = 0;
	
	ht1621_display_symbol(SCREEN_SYMOBOL_M2, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M10, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M9, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M7, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M16, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M17, 1);

	if (process_data.time.reversing_count_down > 60){
		minute = process_data.time.reversing_count_down / 60;
	} else {
		minute = 0;
	}
	
	if (process_data.time.reversing_count_down <= 60){
		second = process_data.time.reversing_count_down;
	} else {
		second = process_data.time.reversing_count_down % 60;
	}

	uint8_t m1 = minute / 10;
	uint8_t m2 = minute % 10;
	uint8_t s1 = second / 10;
	uint8_t s2 = second % 10;

	ht1621_display_number(1, m1);
	ht1621_display_number(2, m2);
	ht1621_display_number(3, s1);
	ht1621_display_number(4, s2);
	ht1621_display_number(5, process_data.station);
}

void screen_display_main_page_station_interval(void)
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M3, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M16, 1);
	
	uint8_t m1 = process_data.time.station_interval_count_down / 10;
	uint8_t m2 = process_data.time.station_interval_count_down % 10;
	ht1621_display_number(3, m1);
	ht1621_display_number(4, m2);
}

void (*screen_main_pages[])(void) = {
	screen_display_realtime,
	screen_display_main_setviewer,
	screen_display_main_setting,
	screen_display_main_reversing,
	screen_display_main_page_station_interval
};

void screen_main_page_switch(enum screen_main_page_e page)
{
	ht1621_clear();
	screen_data.main_page_index = page;
}

void screen_setviewer_page_switch(void)
{
	ht1621_clear();
	screen_data.setviewer_page_index++;
	if (screen_data.setviewer_page_index >= SETVIEWER_PAGE_CNT){
		screen_data.setviewer_page_index = SETVIEWER_REAL_BAR_DIFF;
	}
}

void screen_entry(void *arg)
{
	ht1621b_init();
	screen_main_pages[screen_data.main_page_index]();
	screen_main_page_switch(SCREEN_PAGE_REALTIME);
	while(1){
		screen_main_pages[screen_data.main_page_index]();
		screen_display_battery();
		ht1621_updata();
		rt_thread_mdelay(SCREEN_DISPLAY_PERIOD);		
	}
}

int screen_init(void)
{
	rt_thread_init(&screen_tid,
			"screen",
			screen_entry,
			RT_NULL,
			screen_stack,
			SCREEN_THREAD_STACK_SIZE,
			SCREEN_THREAD_PRIORITY, 
			SCREEN_THREAD_TIMESLICE);
	rt_thread_startup(&screen_tid);
	return 0;
}

  

