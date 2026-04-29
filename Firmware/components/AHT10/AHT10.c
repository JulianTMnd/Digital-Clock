#include "AHT10.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define AHT10_ADDRESS 0x38

typedef struct aht10_context_t {
	i2c_master_dev_handle_t i2c_dev;
} aht10_context_t;

static esp_err_t aht10_i2c_write(aht10_handle_t handle, uint8_t *write_buffer,
								 uint8_t len) {
	aht10_context_t *sensor = (aht10_context_t *)handle;
	esp_err_t err =
		i2c_master_transmit(sensor->i2c_dev, write_buffer, len, 100);
	return err;
}

static esp_err_t aht10_i2c_read(aht10_handle_t handle, uint8_t *read_buffer,
								uint8_t len) {
	aht10_context_t *sensor = (aht10_context_t *)handle;
	esp_err_t err = i2c_master_receive(sensor->i2c_dev, read_buffer, len, 100);
	return err;
}

static esp_err_t aht10_trigger_read(aht10_handle_t handle) {
	uint8_t write_buffer[] = {0xAC, 0x33, 0x00};
	esp_err_t err = aht10_i2c_write(handle, write_buffer, sizeof(write_buffer));
	return err;
}

static esp_err_t aht10_calibrate(aht10_handle_t handle) {
	uint8_t write_buffer[] = {0xE1, 0x08, 0x00};
	esp_err_t err = aht10_i2c_write(handle, write_buffer, sizeof(write_buffer));
	vTaskDelay(pdMS_TO_TICKS(20));
	return err;
}

esp_err_t aht10_init(i2c_master_bus_handle_t bus_handle,
					 aht10_handle_t *ret_handle) {

	if (bus_handle == NULL || ret_handle == NULL)
		return ESP_ERR_INVALID_ARG;


	aht10_context_t *sensor =
		(aht10_context_t *)calloc(1, sizeof(aht10_context_t));
	if (sensor == NULL)
		return ESP_ERR_NO_MEM;


	i2c_device_config_t i2c_device = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = AHT10_ADDRESS,
		.scl_speed_hz = 400000,
	};


	esp_err_t err =
		i2c_master_bus_add_device(bus_handle, &i2c_device, &sensor->i2c_dev);

	if (err != ESP_OK) {
		free(sensor);
		*ret_handle = NULL;
		return err;
	}

	*ret_handle = sensor;
	return err;
}

esp_err_t aht10_deinit(aht10_handle_t handle) {
	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;
	aht10_context_t *sensor = (aht10_context_t *)handle;
	esp_err_t err = i2c_master_bus_rm_device(sensor->i2c_dev);

	free(handle);

	return err;
}

esp_err_t aht10_read(aht10_handle_t handle, float *temperature,
					 float *humidity) {

	if (handle == NULL || temperature == NULL || humidity == NULL)
		return ESP_ERR_INVALID_ARG;

	uint8_t status_byte = 0;
	uint8_t max_retries = 3;
	uint8_t retry_counter = 0;
	bool busy_flag = true;


	esp_err_t err = aht10_i2c_read(handle, &status_byte, sizeof(status_byte));
	if (err != ESP_OK)
		return err;
	bool calibration_flag = (status_byte >> 3) & 1;


	if (!calibration_flag) {
		err = aht10_calibrate(handle);
		if (err != ESP_OK)
			return err;
	}


	err = aht10_trigger_read(handle);
	if (err != ESP_OK)
		return err;


	do {
		err = aht10_i2c_read(handle, &status_byte, sizeof(status_byte));

		if (err != ESP_OK)
			return err;

		busy_flag = (status_byte >> 7) & 1;
		if (!busy_flag)
			break;

		vTaskDelay(pdMS_TO_TICKS(75));
		retry_counter++;

	} while (busy_flag && retry_counter < max_retries);

	if (retry_counter >= max_retries)
		return ESP_ERR_TIMEOUT;


	uint8_t read_buffer[6] = {0};
	err = aht10_i2c_read(handle, read_buffer, sizeof(read_buffer));

	if (err != ESP_OK)
		return err;


	uint32_t raw_humidity = ((uint32_t)read_buffer[1] << 12) |
							(uint32_t)(read_buffer[2] << 4) |
							(read_buffer[3] >> 4);
	uint32_t raw_temperature = (((uint32_t)read_buffer[3] & 0x0F) << 16) |
							   (uint32_t)(read_buffer[4] << 8) | read_buffer[5];

	*temperature = (((float)raw_temperature / 1048576.0f) * 200.0f) - 50.0f;
	*humidity = ((float)raw_humidity / 1048576.0f) * 100;

	return err;
}

esp_err_t aht10_soft_reset(aht10_handle_t handle) {

	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	uint8_t soft_reset = 0xBA;
	esp_err_t err = aht10_i2c_write(handle, &soft_reset, sizeof(soft_reset));
	if (err != ESP_OK)
		return err;
	vTaskDelay(pdMS_TO_TICKS(20));

	err = aht10_calibrate(handle);

	return err;
}
