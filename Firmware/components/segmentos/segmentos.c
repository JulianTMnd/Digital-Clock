
#include "driver/gpio.h"
#include "esp_err.h"
#include "hal/ledc_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/ledc.h"
#include "segmentos.h"

static const int g_display_hex[] = {
	//(G) (F) (E) (D) (C) (B) (DOT) (A)
	0b01111101, // 0
	0b00001100, // 1
	0b10110101, // 2
	0b10011101, // 3
	0b11001100, // 4
	0b11011001, // 5
	0b11111001, // 6
	0b00001101, // 7
	0b11111101, // 8
	 0b11011101, // 9
	0b00000010, // 10 - Dot
	0b00000000, // 11 - Vacio
	0b11101101, // 12 A
	0b11111000, // B
	0b01110001, // C
	
	0b10111100, // D
	0b11110001, // E
	0b11100001, // F
	0b10100000, // R
	
	0b10000000, // DASH
	0b11000101, // DEGREE
};

typedef struct segment7_context_t {
	segment7_config_t config;
	uint8_t *data_buffer;
	uint8_t *dots_buffer;
} segment7_context_t;

static esp_err_t segment7_strobe_pulse(segment7_handle_t handle) {

	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	segment7_context_t *segment = (segment7_context_t *)handle;

	esp_err_t error = gpio_set_level(segment->config.gpio_str, 1);
	if (error != ESP_OK)
		return error;

	error = gpio_set_level(segment->config.gpio_str, 0);
	if (error != ESP_OK)
		return error;

	return error;
}

static esp_err_t segment7_clock_pulse(segment7_handle_t handle) {

	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	segment7_context_t *segment = (segment7_context_t *)handle;

	esp_err_t error = gpio_set_level(segment->config.gpio_clk, 1);
	if (error != ESP_OK)
		return error;

	error = gpio_set_level(segment->config.gpio_clk, 0);
	if (error != ESP_OK)
		return error;

	return error;
}

esp_err_t segment7_init(const segment7_config_t *config,
						segment7_handle_t *ret_handle) {

	if (config == NULL || ret_handle == NULL)
		return ESP_ERR_INVALID_ARG;


	segment7_context_t *segment =
		(segment7_context_t *)calloc(1, sizeof(segment7_context_t));

	if (segment == NULL)
		return ESP_ERR_NO_MEM;


	segment->config = *config;


	segment->data_buffer =
		(uint8_t *)calloc(config->number_digits, sizeof(uint8_t));

	if (segment->data_buffer == NULL) {
		free(segment);
		return ESP_ERR_NO_MEM;
	}


	segment->dots_buffer =
		(uint8_t *)calloc(config->number_digits, sizeof(uint8_t));

	if (segment->dots_buffer == NULL) {
		free(segment->data_buffer);
		free(segment);
		return ESP_ERR_NO_MEM;
	}


	if (segment->config.gpio_clk == GPIO_NUM_NC ||
		segment->config.gpio_data == GPIO_NUM_NC ||
		segment->config.gpio_oe == GPIO_NUM_NC ||
		segment->config.gpio_str == GPIO_NUM_NC) {
		free(segment->dots_buffer);
		free(segment->data_buffer);
		free(segment);
		return ESP_ERR_INVALID_ARG;
	}


	gpio_config_t gpio_cfg = {
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = (1ULL << segment->config.gpio_clk) |
						(1ULL << segment->config.gpio_data) |
						(1ULL << segment->config.gpio_oe) |
						(1ULL << segment->config.gpio_str),
	};

	esp_err_t err = gpio_config(&gpio_cfg);

	if (err != ESP_OK) {
		free(segment->dots_buffer);
		free(segment->data_buffer);
		free(segment);
		return err;
	}

	ledc_timer_config_t ledc_timer = {
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.timer_num = segment->config.ledc_timer,
		.duty_resolution = LEDC_TIMER_8_BIT,
		.freq_hz = 5000,
		.clk_cfg = LEDC_AUTO_CLK,
	};

	ledc_channel_config_t ledc_channel = {
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.channel = segment->config.ledc_channel,
		.timer_sel = segment->config.ledc_timer,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = segment->config.gpio_oe,
		.duty = 0,
		.hpoint = 0

	};

	err = ledc_timer_config(&ledc_timer);
	if (err != ESP_OK) {
		free(segment->dots_buffer);
		free(segment->data_buffer);
		free(segment);
		return err;
	}

	err = ledc_channel_config(&ledc_channel);
	if (err != ESP_OK) {
		free(segment->dots_buffer);
		free(segment->data_buffer);
		free(segment);
		return err;
	}

	*ret_handle = segment;

	return err;
}

esp_err_t segment7_refresh(segment7_handle_t handle) {

	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	segment7_context_t *segment = (segment7_context_t *)handle;

	esp_err_t error;

	for (int current_segment = segment->config.number_digits - 1;
		 current_segment >= 0; current_segment--) {

		uint8_t valor = g_display_hex[segment->data_buffer[current_segment]];

		if (segment->dots_buffer[current_segment]) {
			valor |= g_display_hex[10];
		}

		for (int current_bit = 7; current_bit >= 0; current_bit--) {

			error = gpio_set_level(segment->config.gpio_data,
								   !((valor >> current_bit) & 1));
			if (error != ESP_OK)
				return error;

			error = segment7_clock_pulse(handle);
			if (error != ESP_OK)
				return error;
		}
	}

	error = segment7_strobe_pulse(handle);

	return error;
}

esp_err_t segment7_set_raw(segment7_handle_t handle, int position,
						   uint8_t value, bool show_point) {
	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	if (position >= handle->config.number_digits)
		return ESP_ERR_INVALID_ARG;

	handle->data_buffer[position] = value;

	if (show_point)
		handle->dots_buffer[position] = true;
	else
		handle->dots_buffer[position] = false;

	return ESP_OK;
}

esp_err_t segment7_set_brightness(segment7_handle_t handle,
								  uint8_t brightness) {

	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	segment7_context_t *segment = (segment7_context_t *)handle;

	uint32_t duty_cycle = 255 - brightness;
	esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE,
								  segment->config.ledc_channel, duty_cycle);
	if (err != ESP_OK)
		return err;

	return ledc_update_duty(LEDC_LOW_SPEED_MODE, segment->config.ledc_channel);
}

esp_err_t segment7_deinit(segment7_handle_t handle) {
	if (handle == NULL)
		return ESP_ERR_INVALID_ARG;

	segment7_context_t *segment = (segment7_context_t *)handle;

	esp_err_t final_error = ESP_OK;

	esp_err_t err =
		ledc_stop(LEDC_LOW_SPEED_MODE, segment->config.ledc_channel, 1);

	if (err != ESP_OK) {
		final_error = err;
	}

	err = gpio_reset_pin(segment->config.gpio_clk);
	if (err != ESP_OK) {
		final_error = err;
	}

	err = gpio_reset_pin(segment->config.gpio_data);
	if (err != ESP_OK) {
		final_error = err;
	}
	err = gpio_reset_pin(segment->config.gpio_str);
	if (err != ESP_OK) {
		final_error = err;
	}

	free(handle->dots_buffer);
	free(handle->data_buffer);
	free(handle);

	return final_error;
}

//
