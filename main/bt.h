/*
 * bt.h
 *
 *  Created on: Sep 12, 2022
 *      Author: kosa
 */

#ifndef MAIN_BT_H_
#define MAIN_BT_H_

#include "log.h"
#include "freertos/FreeRTOS.h"


extern TaskHandle_t Led_task_handle;
extern uint8_t led_colour[3];

void init_bt(void);
void send_notify(const measurement_t* meas);


#endif /* MAIN_BT_H_ */
