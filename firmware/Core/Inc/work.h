/**
 * @file work.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef WORK_PROCESS_H
#define WORK_PROCESS_H

#define PROCESS_STATION_INTERVAL 5 // Seconds

enum process_state_t {
    PROCESS_STATE_INIT = 0,
    PROCESS_STATE_STADYING,
    PROCESS_STATE_REVERING,
    PROCESS_STATE_STATION_INTERVAL,
    PROCESS_STATE_SETING,
    PROCESS_STATE_STOP,
};

struct process_time_t{
	int stadying;
	int reversing;
	int reversing_count_down;
	int station_interval;
	int station_interval_count_down;
};

struct process_data_t{
	enum process_state_t state;
	struct process_time_t time;
	int station;
	int reverse_period_num;
};

extern struct process_data_t process_data;

void process_handle(void);
void process_switch(enum process_state_t state);
#endif
