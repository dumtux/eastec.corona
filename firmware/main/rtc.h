#ifndef FIRMWARE_MAIN_RTC_H_
#define FIRMWARE_MAIN_RTC_H_

#include <stdint.h>

void init_time();
void set_time(uint32_t seconds);
uint32_t get_time(void);



#endif /* FIRMWARE_MAIN_RTC_H_ */
