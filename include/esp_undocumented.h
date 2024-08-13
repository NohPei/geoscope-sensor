#ifndef __ESP_UNDOCUMENTED_H__
#define __ESP_UNDOCUMENTED_H__

#if defined(ESP8266)
//Undocumented ESP8266 registers
//Retrieved from https://github.com/esp8266/Arduino/blob/master/tools/sdk/include/eagle_soc.h
#define WDEVTSF0_TIME_LO 			0x3ff21004
#define WDEVTSF0_TIME_HI 			0x3ff21008
#define WDEVTSFSW0_LO 				0x3ff21018
#define WDEVTSFSW0_HI 				0x3ff2101C
#define WDEVTSF0_TIMER_LO 		0x3ff2109c
#define WDEVTSF0_TIMER_HI 		0x3ff210a0
#define WDEVTSF0_TIMER_ENA 		0x3ff21098
#define WDEVTSF0_TIM_EN_MASK 	0x80000000
#define WDEV_COUNT_REG 				0x3ff21004

#define ESP_WDEV_TIMESTAMP() 		(* (volatile int64_t*) WDEVTSF0_TIME_LO)
#define WDEV_TIMESTAMP_INIT() 		(void)0

extern int wDev_MacTimSetFunc(void (*handle)(void));

#elif defined(ESP32)

# include <esp_wifi.h>
# include <esp_mesh.h>
# define ESP_WDEV_TIMESTAMP() 	esp_mesh_get_tsf_time()
# define WDEV_TIMESTAMP_INIT() 	esp_wifi_get_tsf_time((wifi_interface_t)0)

#else

# define ESP_WDEV_TIMESTAMP() 		micros()
# define WDEV_TIMESTAMP_INIT() 		(void)0

#endif


#endif //!defined(__ESP_UNDOCUMENTED_H__)
