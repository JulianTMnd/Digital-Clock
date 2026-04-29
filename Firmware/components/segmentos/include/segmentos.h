

#ifndef _SEGMENTOS_H_
#define _SEGMENTOS_H_

#include "esp_err.h"
#include "soc/gpio_num.h"
#include <stdbool.h>
#include <stdint.h>

#include "driver/ledc.h"

typedef struct segment7_config_t {
	gpio_num_t gpio_oe;
	gpio_num_t gpio_data;
	gpio_num_t gpio_clk;
	gpio_num_t gpio_str;
	uint8_t number_digits;
	ledc_channel_t ledc_channel;
	ledc_timer_t ledc_timer;
} segment7_config_t;

typedef enum {
	SEG_CODE_0 = 0,
	SEG_CODE_1,
	SEG_CODE_2,
	SEG_CODE_3,
	SEG_CODE_4,
	SEG_CODE_5,
	SEG_CODE_6,
	SEG_CODE_7,
	SEG_CODE_8,
	SEG_CODE_9,
	SEG_CODE_DOT,	 // 10
	SEG_CODE_BLANK,	 // 11 (Apagado)
	SEG_CODE_A,		 // 12
	SEG_CODE_B,		 // 13
	SEG_CODE_C,		 // 14
	SEG_CODE_D,		 // 15
	SEG_CODE_E,		 // 16
	SEG_CODE_F,		 // 17
	SEG_CODE_R,		 // 18 (-)
	SEG_CODE_DASH,	 // 19 (°)
	SEG_CODE_DEGREE, // 20 (°)

} segment_glyph_t;

typedef struct segment7_context_t *segment7_handle_t;


/**
 * @brief Initialize a new instance for the 7-segment display.
 * * @param[in] config Configuration structure including GPIOs and PWM settings.
 * @param[out] ret_handle Pointer to the initialized instance handle.
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_NO_MEM: Out of memory
 */

esp_err_t segment7_init(const segment7_config_t *config,
						segment7_handle_t *ret_handle);
/**
 * @brief Update the physical displays with the current buffer data.
 * * @param handle Sensor handle.
 * @return
 * - ESP_OK: Success
 */
esp_err_t segment7_refresh(segment7_handle_t handle);


/* @brief Set the glyph/value to be displayed at a specific position.
 * * @param handle Sensor handle.
 * @param position Index of the target digit (starting from 0).
 * @param value The character or symbol to display (from segment_glyph_t).
 * @param show_point Boolean to turn the decimal point segment ON or OFF.
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_INVALID_ARG: Position out of range
 */
esp_err_t segment7_set_raw(segment7_handle_t handle, int position,
						   uint8_t value, bool show_point);


/**
 * @brief Adjust the display brightness.
 * * @param handle Sensor handle.
 * @param brightness Value from 0 (off) to 255 (maximum brightness).
 * @return
 * - ESP_OK: Success
 */
esp_err_t segment7_set_brightness(segment7_handle_t handle, uint8_t brightness);


/**
 * @brief Deinitialize the 7-segment instance and release resources.
 * * @param handle Sensor handle.
 * @return
 * - ESP_OK: Success
 */
esp_err_t segment7_deinit(segment7_handle_t handle);

#endif
