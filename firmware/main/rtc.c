#include <rtc.h>
#include <time.h>
#include <driver/i2c.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define DS1307_ADDRESS 0x68

static uint8_t intToBCD(uint8_t num) {
	return ((num / 10) << 4) | (num%10);
}

static uint8_t bcdToInt(uint8_t bcd) {
	// 0x10
	return ((bcd >> 4) * 10) + (bcd & 0x0f);;
}

time_t readValue() {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */));
	uint8_t data[7];
	ESP_ERROR_CHECK(i2c_master_read(cmd, data, 6, 0));
	ESP_ERROR_CHECK(i2c_master_read(cmd, &data[6], 1, 1));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	esp_err_t errRc;
	errRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);

	i2c_cmd_link_delete(cmd);
	if (errRc != 0) {
		return 0;
	}

	struct tm tm;
	tm.tm_sec  = bcdToInt(data[0]);
	tm.tm_min  = bcdToInt(data[1]);
	tm.tm_hour = bcdToInt(data[2]);
	tm.tm_mday = bcdToInt(data[4]);
	tm.tm_mon  = bcdToInt(data[5]) - 1; // 0-11 - Note: The month on the DS1307 is 1-12.
	tm.tm_year = bcdToInt(data[6]) + 100; // Years since 1900
	time_t readTime = mktime(&tm);
	return readTime;
}

void writeValue(time_t newTime) {
	struct tm tm;
	gmtime_r(&newTime, &tm);

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_sec), 1));      // seconds
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_min), 1 ));     // minutes
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_hour), 1 ));    // hours
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_wday+1), 1 ));  // week day
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mday), 1));     // date of month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mon+1), 1));    // month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_year-100), 1)); // year
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);

	i2c_cmd_link_delete(cmd);
}

void init_time()
{
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    struct timespec timesp;
    timesp.tv_sec = readValue();
    timesp.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME,&timesp);
}

void set_time(uint32_t seconds)
{
	writeValue(seconds);
    struct timespec timesp;
    timesp.tv_sec = seconds;
    timesp.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME,&timesp);
}

uint32_t get_time(void)
{
    struct timespec time;

    clock_gettime(CLOCK_REALTIME,&time);
    return time.tv_sec;
}
