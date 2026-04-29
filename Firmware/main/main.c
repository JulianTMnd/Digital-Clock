#include "AHT10.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "ds3231.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "hal/ledc_types.h"

#include "segmentos.h"
#include "soc/gpio_num.h"
#include <freertos/queue.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common_types.h"
#include "system_tasks.h"

const char *I2C_MASTER_BUS = "I2C_MASTER";
const char *DEVICE_TAG = "ESP32 INFO";
const char *DEBUG_TAG = "DEBUG";

void get_esp_info() {

	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);

	ESP_LOGI(DEVICE_TAG, "--- Informacion del Hardware ---");
	ESP_LOGI(DEVICE_TAG, "Nucleos (Cores): %d", chip_info.cores);
	ESP_LOGI(DEVICE_TAG, "Modelo (Enum): %d", chip_info.model);
	ESP_LOGI(DEVICE_TAG, "Revision del chip: %d", chip_info.revision);
	ESP_LOGI(DEVICE_TAG, "Caracteristicas (Bitmask): 0x%08X",
			 (unsigned int)chip_info.features);
	ESP_LOGI(DEVICE_TAG, "--------------------------------");
}

esp_err_t init_i2c_bus(i2c_master_bus_handle_t *bus_handle) {
	i2c_master_bus_config_t i2c_master_bus_config = {
		.i2c_port = I2C_NUM_0,
		.sda_io_num = GPIO_NUM_8,
		.scl_io_num = GPIO_NUM_9,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};
	esp_err_t err = i2c_new_master_bus(&i2c_master_bus_config, bus_handle);
	if (err != ESP_OK)
		ESP_LOGE(I2C_MASTER_BUS, "ERROR: bus i2c fallo");
	else
		ESP_LOGI(I2C_MASTER_BUS, "SUCESS: bus i2c creado con exito");

	return err;
}

esp_err_t init_sensors(i2c_master_bus_handle_t bus_handle,
					   ds3231_handle_t *ds3231_handle,
					   aht10_handle_t *aht10_handle) {

	esp_err_t err = aht10_init(bus_handle, aht10_handle);

	if (err != ESP_OK)
		ESP_LOGE(I2C_MASTER_BUS, "ERROR: fallo al agregar AHT10");
	else
		ESP_LOGI(I2C_MASTER_BUS, "SUCESS: AHT10 agregado");

	err = ds3231_init(bus_handle, ds3231_handle);

	if (err != ESP_OK)
		ESP_LOGE(I2C_MASTER_BUS, "ERROR: fallo al agregar DS3231");
	else
		ESP_LOGI(I2C_MASTER_BUS, "SUCESS: DS3231 agregado");

	return err;
}

esp_err_t deinit_sensors(ds3231_handle_t ds3231_handle,
						 aht10_handle_t aht10_handle) {
	esp_err_t err = aht10_deinit(aht10_handle);
	err = ds3231_deinit(ds3231_handle);
	return err;
}

void app_main(void) {

	aht10_handle_t aht10_handle;
	ds3231_handle_t ds3231_handle;
	segment7_handle_t segment_handle;

	i2c_master_bus_handle_t master_bus_handle;

	QueueHandle_t button_queue;
	QueueHandle_t display_queue;

	segment7_config_t segment7_config = {
		.gpio_oe = GPIO_NUM_6,
		.gpio_clk = GPIO_NUM_10,
		.gpio_str = GPIO_NUM_7,
		.gpio_data = GPIO_NUM_5,
		.ledc_channel = LEDC_CHANNEL_0,
		.ledc_timer = LEDC_TIMER_0,
		.number_digits = 4,
	};

	ESP_ERROR_CHECK(segment7_init(&segment7_config, &segment_handle));
	ESP_ERROR_CHECK(segment7_set_brightness(segment_handle, 200));

	ESP_ERROR_CHECK(init_i2c_bus(&master_bus_handle));

	ESP_ERROR_CHECK(
		init_sensors(master_bus_handle, &ds3231_handle, &aht10_handle));


	gpio_config_t gpio_cfg = {
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = (1ULL << GPIO_NUM_4) | (1ULL << GPIO_NUM_3),
	};

	ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

	button_queue = xQueueCreate(10, sizeof(button_event_t));
	if (button_queue == 0) {
		ESP_LOGE(DEBUG_TAG, "ERROR AL CREAR QUEUE BOTONES");
		abort();
	}

	display_queue = xQueueCreate(10, sizeof(system_context_t));
	if (display_queue == 0) {
		ESP_LOGE(DEBUG_TAG, "ERROR AL CREAR QUEUE DISPLAY");
		abort();
	}


	static display_task_params_t displayParamToPass = {0};
	displayParamToPass.display_queue = display_queue;
	displayParamToPass.segments = segment_handle;
	xTaskCreate(display_task, "display_task", 2048, &displayParamToPass, 5,
				NULL);

	static button_task_params_t buttonParamToPass = {0};
	buttonParamToPass.button_queue = button_queue;
	xTaskCreate(button_task, "button_task", 2048, &buttonParamToPass, 3, NULL);

	static sensor_task_params_t sensorParamToPass = {0};
	sensorParamToPass.temp_sensor = aht10_handle;
	sensorParamToPass.button_queue = button_queue;
	sensorParamToPass.display_queue = display_queue;
	sensorParamToPass.rtc = ds3231_handle;
	xTaskCreate(sensor_task, "sensor_task", 4096, &sensorParamToPass, 2, NULL);
}
