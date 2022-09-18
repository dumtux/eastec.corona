/*
 * log.h
 *
 *  Created on: Sep 12, 2022
 *      Author: kosa
 */

#ifndef MAIN_LOG_H_
#define MAIN_LOG_H_

#include <stdint.h>

typedef struct __attribute__((packed)){
	uint32_t sample_in_past; // 0 means current
	uint16_t i_slr; //in 0.01A
	int16_t i_bat; //in 0.01A
	uint16_t v_slr; //in 0.01V
	uint16_t v_bat; // in 0.01V
	uint8_t valid;
} measurement_t;



#endif /* MAIN_LOG_H_ */