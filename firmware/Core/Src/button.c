#include "button.h"
#include <rtthread.h>
#include "main.h"

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
Button_t key_ok;
Button_t key_stop;
Button_t key_back;
static struct button* Head_Button = RT_NULL;
static char *StrnCopy(char *dst, const char *src, rt_uint32_t n);
static void Add_Button(Button_t* btn);

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
  if( btn == RT_NULL)
  {
    RT_DEBUG_LOG(RT_DEBUG_THREAD,("struct button is RT_NULL!"));
  }
  
  if(BUTTON_ALL_RIGGER == btn_event)
  {
    for(rt_uint8_t i = 0 ; i < number_of_event-1 ; i++)
      /*A callback function triggered by a button event ,Used to handle button events */
      btn->CallBack_Function[i] = btn_callback;   
  }
  else
  {
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
int is_long_Trigger = 0;

void Button_Cycle_Process(Button_t *btn)
{
  /* Get the current button level */
  rt_uint8_t current_level = (rt_uint8_t)btn->Read_Button_Level();
  
  /* Button level changes, debounce */
  if((current_level != btn->Button_Last_Level)&&(++(btn->Debounce_Time) >= BUTTON_DEBOUNCE_TIME))
  {
      /* Update current button level */
      btn->Button_Last_Level = current_level;

      /* button is pressed */
      btn->Debounce_Time = 0;
      
      /* If the button is not pressed, change the button state to press (first press / double trigger) */
      if((btn->Button_State == NONE_TRIGGER)||(btn->Button_State == BUTTON_DOUBLE))
      {
        btn->Button_State = BUTTON_DOWM;
      }
      //free button
      else if(btn->Button_State == BUTTON_DOWM)
      {
        btn->Button_State = BUTTON_UP;
        RT_DEBUG_LOG(RT_DEBUG_THREAD,("button release"));
      }
  }
  
  switch(btn->Button_State)
  {
    /* button dowm */
    case BUTTON_DOWM :                                        
    {
      if(btn->Button_Last_Level == btn->Button_Trigger_Level) 
      {
        /* Support continuous triggering */
        #ifdef CONTINUOS_TRIGGER                              

        if(++(btn->Button_Cycle) >= BUTTON_CONTINUOS_CYCLE)
        {
          btn->Button_Cycle = 0;
          btn->Button_Trigger_Event = BUTTON_CONTINUOS; 
          /* continuous triggering */
          TRIGGER_CB(BUTTON_CONTINUOS);                      
          RT_DEBUG_LOG(RT_DEBUG_THREAD,("continuous triggering"));
        }
        
        #else
        
        btn->Button_Trigger_Event = BUTTON_DOWM;
        
        /* Update the trigger event before releasing the button as long press */
        if(++(btn->Long_Time) >= BUTTON_LONG_TIME)            
        {
          #ifdef LONG_FREE_TRIGGER
          if (btn->long_func_called == 0){
		btn->long_func_called = 1;
		TRIGGER_CB(BUTTON_LONG);    
	  }
          btn->Button_Trigger_Event = BUTTON_LONG; 
          
          #else
          
          /* Continuous triggering of long press cycles */
          if(++(btn->Button_Cycle) >= BUTTON_LONG_CYCLE)      
          {
            btn->Button_Cycle = 0;
            btn->Button_Trigger_Event = BUTTON_LONG; 
            
            /* long triggering */
            TRIGGER_CB(BUTTON_LONG);    
          }
          #endif
          
          /* Update time overflow */
          if(btn->Long_Time == 0xFF)
          {
            btn->Long_Time = BUTTON_LONG_TIME;
          }
          RT_DEBUG_LOG(RT_DEBUG_THREAD,("Long press"));
        }
          
        #endif
      }

      break;
    } 
    
    /* button up */
    case BUTTON_UP :
    {
      /* Trigger click */
      if(btn->Button_Trigger_Event == BUTTON_DOWM)          
      {
        /* double click */
        if((btn->Timer_Count <= BUTTON_DOUBLE_TIME)&&(btn->Button_Last_State == BUTTON_DOUBLE)) 
        {
          btn->Button_Trigger_Event = BUTTON_DOUBLE;
          TRIGGER_CB(BUTTON_DOUBLE);    
          RT_DEBUG_LOG(RT_DEBUG_THREAD,("double click"));
          btn->Button_State = NONE_TRIGGER;
          btn->Button_Last_State = NONE_TRIGGER;
        }
        else
        {
            btn->Timer_Count=0;
            /* Detection long press failed, clear 0 */
            btn->Long_Time = 0;
          
          #ifndef SINGLE_AND_DOUBLE_TRIGGER

             /* click */
            TRIGGER_CB(BUTTON_DOWM);
          #endif
            btn->Button_State = BUTTON_DOUBLE;
            btn->Button_Last_State = BUTTON_DOUBLE;
          
        }
      }
      
      else if(btn->Button_Trigger_Event == BUTTON_LONG)
      {
        #ifdef LONG_FREE_TRIGGER
          /* Long press */
          //TRIGGER_CB(BUTTON_LONG);
	      btn->long_func_called = 0;
        #else
          
          /* Long press free */
          TRIGGER_CB(BUTTON_LONG_FREE);
        #endif
        btn->Long_Time = 0;
        btn->Button_State = NONE_TRIGGER;
        btn->Button_Last_State = BUTTON_LONG;
      } 
      
      #ifdef CONTINUOS_TRIGGER
        /* Press continuously */
        else if(btn->Button_Trigger_Event == BUTTON_CONTINUOS)  
        {
          btn->Long_Time = 0;
           /* Press continuously free */
          TRIGGER_CB(BUTTON_CONTINUOS_FREE);
          btn->Button_State = NONE_TRIGGER;
          btn->Button_Last_State = BUTTON_CONTINUOS;
        } 
      #endif
      
      break;
    }
    
    case BUTTON_DOUBLE :
    {
      /* Update time */
      btn->Timer_Count++;                                      
      if(btn->Timer_Count>=BUTTON_DOUBLE_TIME)
      {
        btn->Button_State = NONE_TRIGGER;
        btn->Button_Last_State = NONE_TRIGGER;
      }
      #ifdef SINGLE_AND_DOUBLE_TRIGGER
      
        if((btn->Timer_Count>=BUTTON_DOUBLE_TIME)&&(btn->Button_Last_State != BUTTON_DOWM))
        {
          btn->Timer_Count=0;
          TRIGGER_CB(BUTTON_DOWM);    
          btn->Button_State = NONE_TRIGGER;
          btn->Button_Last_State = BUTTON_DOWM;
        }
        
      #endif

      break;
    }

    default :
      break;
  }
  
}

void Button_Process(void)
{
  struct button* pass_btn;
  for(pass_btn = Head_Button; pass_btn != RT_NULL; pass_btn = pass_btn->Next)
  {
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
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY3_UP_GPIO_Port, KEY3_UP_Pin);
}

rt_uint8_t key_down_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY2_DOWN_GPIO_Port, KEY2_DOWN_Pin);
}

rt_uint8_t key_ok_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY1_OK_GPIO_Port, KEY1_OK_Pin);
}

rt_uint8_t key_stop_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY4_STOP_GPIO_Port, KEY4_STOP_Pin);
}

rt_uint8_t key_back_read_level(void)
{
	return (rt_uint8_t)HAL_GPIO_ReadPin(KEY0_BACK_GPIO_Port, KEY0_BACK_Pin);
}

void key_up_down_callback(void *arg)
{
        
}
void key_down_down_callback(void *arg)
{
	
}
void key_ok_down_callback(void *arg)
{
        
}
void key_stop_down_callback(void *arg)
{
        
}
void key_back_down_callback(void *arg)
{
        
}

void button_entry(void *arg)
{
	Button_Create("key_up", &key_up, key_up_read_level, 0);
	Button_Attach(&key_up, BUTTON_DOWM, key_up_down_callback);
	Button_Create("key_down", &key_down, key_down_read_level, 0);
	Button_Attach(&key_down, BUTTON_DOWM, key_down_down_callback);
	Button_Create("key_ok", &key_ok, key_ok_read_level, 0);
	Button_Attach(&key_ok, BUTTON_DOWM, key_ok_down_callback);
	Button_Create("key_stop", &key_stop, key_stop_read_level, 0);
	Button_Attach(&key_stop, BUTTON_DOWM, key_stop_down_callback);
	Button_Create("key_back", &key_back, key_back_read_level, 0);
	Button_Attach(&key_back, BUTTON_DOWM, key_back_down_callback);
	while(1){
		Button_Process();
		rt_thread_mdelay(20);
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



