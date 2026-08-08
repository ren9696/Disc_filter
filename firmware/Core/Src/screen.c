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
#define SCREEN_SYMOBOL_CNT 19+1
#define SCREEN_RAM_BUFF_SIZE (32)
uint8_t screen_display_ram[SCREEN_RAM_BUFF_SIZE]; 
struct screen_data_t screen_data;

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
	(15 << 4) | 1
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

static void screen_display_reverse_num(uint8_t num)
{
	if (num > 99){
		num = 99;
	}
	uint8_t num1 = num / 10;
	uint8_t num2 = num % 10;
	ht1621_display_number(3, num1);
	ht1621_display_number(4, num2);
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

	uint8_t h1 = 13 / 10;
	uint8_t h2 = 13 % 10;
	uint8_t m1 = 45 / 10;
	uint8_t m2 = 45 % 10;
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
			c-=1;				// 同上 
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

void ht1621b_init(void)
{
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


static void screen_display_state_real_bar_diff(void)
{
	ht1621_display_symbol(SCREEN_SYMOBOL_M12, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M13, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M18, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M19, 1);
	ht1621_display_p(1, 1);
	ht1621_display_p(2, 1);
	screen_display_real_bardiff_num(12);
	screen_display_reverse_num(45);
}

static void screen_display_realtime()
{
	screen_display_state_real_bar_diff();
}

static void screen_setviewer_real_bar_diff()
{
        ht1621_display_symbol(SCREEN_SYMOBOL_M12, 1);
	ht1621_display_symbol(SCREEN_SYMOBOL_M18, 1);
	ht1621_display_p(1, 1);
	screen_display_real_bardiff_num(12);

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
	screen_display_reverse_num(0);
}

static void screen_setviewer_know()
{
	screen_display_real_bardiff_num(0);
	screen_display_reverse_num(0);
}

static void screen_display_setviewer()
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

void (*screen_main_pages[])(void) = {
	screen_page_none,
	screen_display_realtime,
	screen_display_setviewer
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
	screen_power_ctrl(0);
	rt_thread_mdelay(100);
	ht1621b_init();
	screen_main_pages[screen_data.main_page_index]();
	screen_main_page_switch(SCREEN_PAGE_REALTIME);
	while(1){
		screen_main_pages[screen_data.main_page_index]();
		ht1621_updata();
		rt_thread_mdelay(100);		
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

  

