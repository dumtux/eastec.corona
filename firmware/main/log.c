
#include "log.h"

#include <stdbool.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_partition.h"
#include "esp_log.h"

static bool log_initialized = true;
static uint32_t current_sample; // not_yet_written
static uint32_t latched_sample;

static const esp_partition_t *partition;

typedef struct __attribute__((packed)){
	uint8_t valid;
	uint32_t date; // seconds from EPOCH
	uint16_t i_slr; //in 0.01A
	int16_t i_bat; //in 0.01A
	uint16_t v_slr; //in 0.01V
	uint16_t v_bat; // in 0.01V
} flash_entry;

static const uint32_t log_entry_size = 16;

static flash_entry get_raw_entry(uint32_t raw_sample)
{
	flash_entry ret;
    ESP_ERROR_CHECK(esp_partition_read(partition, raw_sample*log_entry_size, &ret, sizeof(ret)));
	return ret;
}

static void set_raw_entry(uint32_t raw_sample, flash_entry *entry)
{
    ESP_ERROR_CHECK(esp_partition_write(partition, raw_sample*log_entry_size, entry, sizeof(*entry)));
}

void init_sample(void)
{
    partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    assert(partition != NULL);

    uint32_t read_size=0;
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    nvs_get_u32(my_handle, "part_size", &read_size);

    if(read_size != partition->size)
    {
        nvs_set_u32(my_handle, "part_size", partition->size);
        ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
    }

    current_sample=0;
    for(;current_sample<partition->size/log_entry_size;current_sample++)
    {
    	if(get_raw_entry(current_sample).valid == 0x55)
    		break;
    }
    for(;current_sample<partition->size/log_entry_size;current_sample++)
    {
    	if(get_raw_entry(current_sample).valid != 0x55)
    		break;
    }
    current_sample = current_sample % (partition->size/log_entry_size);
    log_initialized = true;
}

void add_sample(const measurement_t * sample)
{
	if((current_sample+1)%(SPI_FLASH_SEC_SIZE/log_entry_size) == 0)
	{
		uint32_t sec_to_erased = ((current_sample+1) * log_entry_size) % partition->size;
	    ESP_ERROR_CHECK(esp_partition_erase_range(partition, sec_to_erased, SPI_FLASH_SEC_SIZE));
	}
	flash_entry entry;
	entry.valid = 0x55;
	entry.date = sample->date;
	entry.i_bat = sample->i_bat;
	entry.i_slr = sample->i_slr;
	entry.v_bat = sample->v_bat;
	entry.v_slr = sample->v_slr;
	set_raw_entry(current_sample,&entry);
    current_sample = (current_sample +1) % (partition->size/log_entry_size);
}

void get_sample(uint32_t sample_idx, measurement_t * sample)
{
	if(sample_idx > ((partition->size-SPI_FLASH_SEC_SIZE)/log_entry_size))
	{
		measurement_t meas = {sample_idx,0,101,201,301,401,2};
		*sample = meas;
		return;
	}
	sample->sample_in_past = sample_idx;
    if(latched_sample<sample_idx)
    {
        sample_idx -=partition->size/log_entry_size;
    }
	uint32_t raw_offset = (latched_sample - sample_idx)%(partition->size/log_entry_size);
	flash_entry entry = get_raw_entry(raw_offset);
	sample->valid = (entry.valid==0x55) ? 1 : 0;
	sample->date = entry.date;
	sample->i_bat = entry.i_bat;
	sample->i_slr = entry.i_slr;
	sample->v_bat = entry.v_bat;
	sample->v_slr = entry.v_slr;
}

void latch_sample(void)
{
	latched_sample = current_sample;
}
