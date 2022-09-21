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
	uint32_t date; // seconds from EPOCH
	uint16_t i_slr; //in 0.01A
	int16_t i_bat; //in 0.01A
	uint16_t v_slr; //in 0.01V
	uint16_t v_bat; // in 0.01V
	uint8_t valid;
} measurement_t;


void init_sample(void);
void add_sample(const measurement_t * sample);
void get_sample(uint32_t sample_idx, measurement_t * sample);
void latch_sample(void);

#endif /* MAIN_LOG_H_ */
