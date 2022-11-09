
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include <rtc.h>

#include "led_strip.h"

#include "bt.h"

#include "sdkconfig.h"

TaskHandle_t Led_task_handle = NULL;
uint8_t led_colour[3];

static led_strip_t *led_strip;

#define MAX_LEDS 30

void led_setup_task(void* arg)
{
    led_strip = led_strip_init(0, 10, MAX_LEDS);
    led_strip->clear(led_strip,MAX_LEDS);
	while(true)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		for(int i=0;i<MAX_LEDS;i++)
			led_strip->set_pixel(led_strip, i, led_colour[2], led_colour[1], led_colour[0]);
		led_strip->refresh(led_strip,MAX_LEDS);
	}
}

void app_main(void)
{
    esp_err_t ret;

	gpio_config_t chg_io_conf = {
			.pin_bit_mask = (1 << 3),
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = 0,
			.pull_down_en = 0,
			.intr_type =GPIO_INTR_DISABLE,
	};

    gpio_config(&chg_io_conf);
    gpio_set_level(3, 0);

	gpio_config_t inp_io_conf = {
			.pin_bit_mask = (1 << 6)|(1 << 7),
			.mode = GPIO_MODE_DISABLE,
			.pull_up_en = 0,
			.pull_down_en = 0,
			.intr_type =GPIO_INTR_DISABLE,
	};

    gpio_config(&inp_io_conf);

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    xTaskCreate(led_setup_task, "LED_STRIP", 4000, NULL, 5, &Led_task_handle );

    init_time();

    init_sample();

    init_bt();

    static esp_adc_cal_characteristics_t adc1_chars,adc2_chars;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc2_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11));

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    uint32_t log_div_counter = 0;

    while(true)
    {
        uint32_t v_slr_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc1_chars);
        uint32_t v_bat_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_1), &adc1_chars);
        uint32_t i_slr_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_4), &adc1_chars);
        int adc2_raw;
        adc2_get_raw(ADC1_CHANNEL_0,ADC_WIDTH_BIT_DEFAULT, &adc2_raw);
        int32_t i_bat_mv = esp_adc_cal_raw_to_voltage(adc2_raw, &adc2_chars);

        //for now raw voltages
        uint16_t i_slr = (float)i_slr_mv /(0.033*50*1000/100);
        int16_t i_bat = (float)(i_bat_mv - 1650) /(0.033*50*1000/100);
        uint16_t v_slr = v_slr_mv /10;
        uint16_t v_bat = v_bat_mv /10;


    	measurement_t meas = {0,get_time(),i_slr,i_bat,v_slr,v_bat,1};
    	if(log_div_counter == 0)
    		add_sample(&meas);
    	log_div_counter++;
    	if(log_div_counter == 60*5)
    		log_div_counter = 0;
    	send_notify(&meas);
    	vTaskDelayUntil(&xLastWakeTime,1000/portTICK_PERIOD_MS);
    }

    return;
}
