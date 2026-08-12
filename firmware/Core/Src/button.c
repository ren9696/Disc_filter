#include "button.h"
#include <rtthread.h>
#include "main.h"
#include "screen.h"
#include "beep.h"
#include "save.h"

#define PKG_USING_BUTTON 
#ifdef PKG_USING_BUTTON
static struct rt_thread button_tid;
#define BUTTON_THREAD_STACK_SIZE 512
#define BUTTON_THREAD_PRIORITY 15
#define BUTTON_THREAD_TIMESLICE 20
static char button_stack[BUTTON_THREAD_STACK_SIZE];
INIT_APP_EXPORT(Button_init);   
Button_t key_up;
Button_t key_down;
Button_t key_entry;
Button_t key_back;
Button_t manual;
static struct button* Head_Button = RT_NULL;
static char *StrnCopy(char *dst, const char *src, rt_uint32_t n);
static void Add_Button(Button_t* btn);
static rt_uint8_t Timer_Count2_en;                       /* 2Timing enable*/
void Button_Create(const char *name,
                  Button_t *btn, 
                  rt_uint8_t(*read_btn_level)(void),
                  rt_uint8_t btn_trigger_level)
{
  if( btn == RT_NULL)
  {
    RT_DEBUG_LOG(RT_DEBUG_THREAD,("struct button is RT_NULL!"));
  }
  
  memset(btn, 0, sizeof(struct button));      //Clear structure information
 
  StrnCopy(btn->Name, name, BTN_NAME_MAX);    //button name 
  
  btn->Button_State = NONE_TRIGGER;                     //Button status
  btn->Button_Last_State = NONE_TRIGGER;                //Button last status
  btn->Button_Trigger_Event = NONE_TRIGGER;             //Button trigger event
  btn->Read_Button_Level = read_btn_level;              //Button trigger level reading function
  btn->Button_Trigger_Level = btn_trigger_level;        //Button trigger level
  btn->Button_Last_Level = btn->Read_Button_Level();    //Button current level
  btn->Debounce_Time = 0;
  
  RT_DEBUG_LOG(RT_DEBUG_THREAD,("button create success!"));
  
  Add_Button(btn);          //Added to the singly linked list when button created 
}

void Button_Attach(Button_t *btn,Button_Event btn_event,Button_CallBack btn_callback)
{
	if ( btn == RT_NULL){
		RT_DEBUG_LOG(RT_DEBUG_THREAD,("struct button is RT_NULL!"));
	}
  
	if (btn_event == BUTTON_CONTINUOS){
		btn->hold_mode = HOLD_MODE_CONTINUOS;
	} else if (btn_event == BUTTON_LONG){
		btn->hold_mode = HOLD_MODE_LONG;
	}

	if (BUTTON_ALL_RIGGER == btn_event){

		/*A callback function triggered by a button event ,Used to handle button events */
		for(rt_uint8_t i = 0 ; i < number_of_event-1 ; i++)
			btn->CallBack_Function[i] = btn_callback;   
			
	} else {
		btn->CallBack_Function[btn_event] = btn_callback; 
	}
	
}

void Button_Delete(Button_t *btn)
{
  struct button** curr;
  for(curr = &Head_Button; *curr;) 
  {
    struct button* entry = *curr;
    if (entry == btn) 
    {
      *curr = entry->Next;
    } 
    else
    {
      curr = &entry->Next;
    }
  }
}

rt_uint8_t Get_Button_Event(Button_t *btn)
{
  return (rt_uint8_t)(btn->Button_Trigger_Event);
}


rt_uint8_t Get_Button_State(Button_t *btn)
{
  return (rt_uint8_t)(btn->Button_State);
}


void Button_SetTriggerTime(Button_t *btn, rt_uint8_t trigger_time)
{
  if (btn == RT_NULL){
    RT_DEBUG_LOG(RT_DEBUG_THREAD,("struct button is RT_NULL!"));
    return;
  }
  btn->trigger_time = trigger_time;
}

int is_long_Trigger = 0;
void Button_Cycle_Process(Button_t *btn)
{
	rt_uint8_t current_level = (rt_uint8_t)btn->Read_Button_Level();
  
	if ( (current_level != btn->Button_Last_Level) && (++(btn->Debounce_Time) >= BUTTON_DEBOUNCE_TIME) ){
		btn->Button_Last_Level = current_level;
		btn->Debounce_Time = 0;
		
			if (current_level== btn->Button_Trigger_Level){
				btn->Button_State = BUTTON_DOWM;
				btn->Button_Trigger_Event = BUTTON_DOWM;

				/* 按下触发：按下瞬间即触发单击 */
				if (btn->trigger_time == TRIGGER_ON_PRESS){
					TRIGGER_CB(BUTTON_DOWM);
				}
			} else {
				btn->Button_State = BUTTON_UP;
			}
	}
	
	switch(btn->Button_State){
	case BUTTON_DOWM :                                        
		if(btn->Button_Last_Level != btn->Button_Trigger_Level) break;

		switch (btn->hold_mode){
		case HOLD_MODE_CONTINUOS :
			if(++(btn->Timer_Count) >= BUTTON_CONTINUOS_DELAY){
				btn->Timer_Count = BUTTON_CONTINUOS_DELAY;
				if(++(btn->Button_Cycle) >= BUTTON_CONTINUOS_CYCLE){
					btn->Button_Cycle = 0;
					btn->Button_Trigger_Event = BUTTON_CONTINUOS;
					TRIGGER_CB(BUTTON_CONTINUOS);
				}
				if (++(btn->Timer_Count2) >= BUTTON_CONTINUOS_DELAY2){
					btn->Timer_Count2 = BUTTON_CONTINUOS_DELAY2;
					Timer_Count2_en = 1;
				}
			}
			break;
			
		case HOLD_MODE_LONG :
			if(++(btn->Long_Time) >= BUTTON_LONG_TIME){
				btn->Long_Time = BUTTON_LONG_TIME;
				if(btn->long_func_called == 0){
					btn->long_func_called = 1;
					btn->Button_Trigger_Event = BUTTON_LONG;
					TRIGGER_CB(BUTTON_LONG);
				}
			}
			break;
		}
		break;
    
		/* button up */
	case BUTTON_UP :
		/* 抬起触发：抬起瞬间才触发单击；若本次发生过长按，则不再触发单击 */
		if ((btn->trigger_time == TRIGGER_ON_RELEASE) && (btn->long_func_called == 0)){
			TRIGGER_CB(BUTTON_DOWM);
		}
		btn->Button_Cycle = 0;
		btn->Timer_Count=0;
		btn->Timer_Count2 = 0;
		btn->Long_Time = 0;
		btn->long_func_called = 0;
		Timer_Count2_en = 0;
		btn->Button_State = NONE_TRIGGER;
        	btn->Button_Last_State = NONE_TRIGGER;
      		break;

	default :
		break;	
  }
  
}

void Button_Process(void)
{
	struct button* pass_btn;
	for(pass_btn = Head_Button; pass_btn != RT_NULL; pass_btn = pass_btn->Next){
		Button_Cycle_Process(pass_btn);
	}
}

void Search_Button(void)
{
  struct button* pass_btn;
  for(pass_btn = Head_Button; pass_btn != RT_NULL; pass_btn = pass_btn->Next)
  {
    RT_DEBUG_LOG(RT_DEBUG_THREAD,("button node have %s",pass_btn->Name));
  }
}

static char *StrnCopy(char *dst, const char *src, rt_uint32_t n)
{
  if (n != 0)
  {
    char *d = dst;
    const char *s = src;
    do
    {
        if ((*d++ = *s++) == 0)
        {
            while (--n != 0)
                *d++ = 0;
            break;
        }
    } while (--n != 0);
  }
  return (dst);
}

static void Add_Button(Button_t* btn)
{
  struct button *pass_btn = Head_Button;
  
  while(pass_btn)
  {
    pass_btn = pass_btn->Next;
  }
  
  btn->Next = Head_Button;
  Head_Button = btn;
}

rt_uint8_t key_up_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY3_S4_GPIO_Port, KEY3_S4_Pin);
}

rt_uint8_t key_down_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY2_S3_GPIO_Port, KEY2_S3_Pin);
}

rt_uint8_t key_entry_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY1_S2_GPIO_Port, KEY1_S2_Pin);
}

rt_uint8_t key_back_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY4_S5_GPIO_Port, KEY4_S5_Pin);
}

rt_uint8_t key_manual_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY0_S1_GPIO_Port, KEY0_S1_Pin);
}

void key_setting_page_rever_period_handle(uint8_t direction)
{
	if (direction){
		if (Timer_Count2_en){
			save_set_data.rever_period_m+= 10;
		} else {
			save_set_data.rever_period_m++;
		}
		
		if (save_set_data.rever_period_m > 59){
			save_set_data.rever_period_m = 0;
			save_set_data.rever_period_h++;
			if (save_set_data.rever_period_h > 24){
				save_set_data.rever_period_h = 0;
			}
		}
	} else {
		if (save_set_data.rever_period_m <= 0){
			save_set_data.rever_period_m = 59;
			if (save_set_data.rever_period_h <= 0){
				save_set_data.rever_period_h = 24;	
			} else {
				save_set_data.rever_period_h--;
			}
		} else {
			if (Timer_Count2_en){
				save_set_data.rever_period_m-= 10;
			} else {
				save_set_data.rever_period_m--;
			}
		}
	}
}

void key_setting_page_rever_time_handle(uint8_t direction)
{
	if (direction){
		if (Timer_Count2_en){
			save_set_data.rever_time_s+= 10;
		} else {
			save_set_data.rever_time_s++;
		}
		
		if (save_set_data.rever_time_s > 59){
			save_set_data.rever_time_s = 0;
			save_set_data.rever_time_m++;
			if (save_set_data.rever_time_m > 59){
				save_set_data.rever_time_m = 0;
			}
		}
	} else {
		if (save_set_data.rever_time_s <= 0){
			save_set_data.rever_time_s = 59;
			if (save_set_data.rever_time_m <= 0){
				save_set_data.rever_time_m = 60;
			} else {
				save_set_data.rever_time_m--;
			}
		} else {
			if (Timer_Count2_en){
				save_set_data.rever_time_s-= 10;
			} else {
				save_set_data.rever_time_s--;
			}
			
		}
	}
}

void key_setting_page_station_interval_handle(uint8_t direction)
{
	if (direction){
		save_set_data.station_interval++;
		if (save_set_data.station_interval > 99){
		        save_set_data.station_interval = 0;
		}
	} else {
		if (save_set_data.station_interval <= 0){
			save_set_data.station_interval = 99;
		} else {
			save_set_data.station_interval--;
		}
	}
}

void key_setting_page_rever_bar_diff_handle(uint8_t direction)
{
	if (direction){
		save_set_data.rever_bar_diff++;
		if (save_set_data.rever_bar_diff > 99){
			save_set_data.rever_bar_diff = 0;
		}
	} else {
		if (save_set_data.rever_bar_diff <= 0){
			save_set_data.rever_bar_diff = 99;
		} else {
			save_set_data.rever_bar_diff--;
		}
	}
}

void key_setting_page_main_valve_handle(uint8_t direction)
{
	if (direction){
		save_set_data.main_valve++;
		if (save_set_data.main_valve > 99){
			save_set_data.main_valve = 0;
		}
	} else {
		if (save_set_data.main_valve <= 0){
			save_set_data.main_valve = 99;
		} else {
			save_set_data.main_valve--;
		}
	}
}

void key_setting_page_station_num_handle(uint8_t direction)
{
	if (direction){
		save_set_data.station_num++;
		if (save_set_data.station_num > 99){
			save_set_data.station_num = 0;
		}
	} else {
		if (save_set_data.station_num <= 0){
			save_set_data.station_num = 99;
		} else {
			save_set_data.station_num--;
		}
	}
}

/**
 * @brief 按键增减
 * 
 * @param data 1:增加 0:减少
 */
void key_ctrl_data(uint8_t data)
{
	switch (screen_data.setting_page_index){
	case SETING_PAGE_REVER_PERIOD:
		key_setting_page_rever_period_handle(data);
		break;

	case SETING_PAGE_REVER_TIME:
		key_setting_page_rever_time_handle(data);
		break;

	case SETING_PAGE_STATION_INTERVAL:
		key_setting_page_station_interval_handle(data);
		break;

	case SETING_PAGE_REVER_BAR_DIFF:
		key_setting_page_rever_bar_diff_handle(data);
		break;

	case SETING_PAGE_MAIN_VALVE:
		key_setting_page_main_valve_handle(data);
		break;

	case SETING_PAGE_STATION_NUM:
		key_setting_page_station_num_handle(data);
		break;

	default:
		return;
	}

	beep_send(BEEP_MSG_TYPE_1);
}

void key_up_down_callback(void *arg)
{
	key_ctrl_data(1);
}

void key_up_down_contin_callback(void *arg)
{       
	key_ctrl_data(1);
}

void key_down_contin_callback(void *arg)
{
	key_ctrl_data(0);
}

void key_down_down_callback(void *arg)
{
	
	key_ctrl_data(0);
}

void key_ok_entry_callback(void *arg)
{
        switch (screen_data.main_page_index)
	{
		case SCREEN_PAGE_REALTIME:
			screen_main_page_switch(SCREEN_PAGE_SETVIEWER);
			screen_data.setviewer_page_index = SETVIEWER_REAL_BAR_DIFF;
			break;

		case SCREEN_PAGE_SETVIEWER:
			screen_setviewer_page_switch();
			break;

		case SCREEN_PAGE_SETING:
			screen_seting_page_switch();
			break;

		default:
			return;
	}

	beep_send(BEEP_MSG_TYPE_1);
}

void key_ok_entry_long_callback(void *arg)
{
	switch (screen_data.main_page_index)
	{
		case SCREEN_PAGE_REALTIME:
			beep_send(BEEP_MSG_TYPE_2);
			screen_main_page_switch(SCREEN_PAGE_SETING);
			screen_data.setting_page_index = SETING_PAGE_REVER_PERIOD;
			break;

		default:
			break;
	}
}

void key_back_down_callback(void *arg)
{
	switch (screen_data.main_page_index)
	{
		case SCREEN_PAGE_REALTIME:
			break;

		case SCREEN_PAGE_SETING:
		case SCREEN_PAGE_SETVIEWER:
			screen_main_page_switch(SCREEN_PAGE_REALTIME);
			break;

		default:
			return;
	}

	beep_send(BEEP_MSG_TYPE_1);
}

void key_manual_down_callback(void *arg)
{
        beep_send(BEEP_MSG_TYPE_1);
}

void button_entry(void *arg)
{
	Button_Create("key_up", &key_up, key_up_read_level, 0);
	Button_Attach(&key_up, 	BUTTON_DOWM, key_up_down_callback);
	Button_Attach(&key_up, 	BUTTON_CONTINUOS, key_up_down_contin_callback);
	Button_SetTriggerTime(&key_up, TRIGGER_ON_PRESS);

	Button_Create("key_down", &key_down, key_down_read_level, 0);
	Button_Attach(&key_down, BUTTON_DOWM, key_down_down_callback);
	Button_Attach(&key_down, BUTTON_CONTINUOS, key_down_contin_callback);
	Button_SetTriggerTime(&key_down, TRIGGER_ON_PRESS);   

	Button_Create("key_entry", &key_entry, key_entry_read_level, 0);
	Button_Attach(&key_entry, BUTTON_DOWM, key_ok_entry_callback);
	Button_Attach(&key_entry, BUTTON_LONG, key_ok_entry_long_callback);
	Button_SetTriggerTime(&key_entry, TRIGGER_ON_RELEASE);

	Button_Create("key_back", &key_back, key_back_level, 0);
	Button_Attach(&key_back, BUTTON_DOWM, key_back_down_callback);
	Button_SetTriggerTime(&key_back, TRIGGER_ON_PRESS);   
	
	Button_Create("manual", &manual, key_manual_read_level, 0);
	Button_Attach(&manual, BUTTON_DOWM, key_manual_down_callback);
	Button_SetTriggerTime(&manual, TRIGGER_ON_PRESS);     

	while(1){
		Button_Process();
		rt_thread_mdelay(BUTTON_CALL_CYCLE);
	}
}

int Button_init(void)
{
	rt_thread_init(&button_tid,
			"button",
			button_entry,
			RT_NULL,
			button_stack,
			BUTTON_THREAD_STACK_SIZE,
			BUTTON_THREAD_PRIORITY, 
			BUTTON_THREAD_TIMESLICE);
	rt_thread_startup(&button_tid);
	return 0;
}

  
#endif



