#ifndef _ESP_UNDOCUMENTED_H_
#define _ESP_UNDOCUMENTED_H_

#include<stdint.h>

#define WDEVTSF0_TIME_LO 			0x3ff21004
#define WDEVTSF0_TIME_HI 			0x3ff21008
#define WDEVTSFSW0_LO 				0x3ff21018
#define WDEVTSFSW0_HI 				0x3ff2101C
#define WDEVTSF0_TIMER_LO 		0x3ff2109c
#define WDEVTSF0_TIMER_HI 		0x3ff210a0
#define WDEVTSF0_TIMER_ENA 		0x3ff21098
#define WDEVTSF0_TIM_EN_MASK 	0x80000000	
#define WDEV_COUNT_REG 				0x3ff21004

#define ESP_WDEV_TIMESTAMP() 		(* (volatile uint64_t*) WDEVTSF0_TIME_LO)

extern int wDev_MacTimSetFunc(void (*handle)(void));


#endif //define _ESP_UNDOCUMENTED_H_
