
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "led_strip.h"

#include "bt.h"

#include "sdkconfig.h"

TaskHandle_t Led_task_handle = NULL;
uint8_t led_colour[3];

static led_strip_t *led_strip;

void led_setup_task(void* arg)
{
    led_strip = led_strip_init(0, 10, 1);
    led_strip->clear(led_strip,1);
	while(true)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		led_strip->set_pixel(led_strip, 0, led_colour[2], led_colour[1], led_colour[0]);
		led_strip->refresh(led_strip,1);
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

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    xTaskCreate(led_setup_task, "LED_STRIP", 4000, NULL, 5, &Led_task_handle );

    init_bt();


    while(true)
    {
    	measurement_t meas = {0,100,200,300,400,1};
    	send_notify(&meas);
    	vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    return;
}
