
#ifndef DS3231_H
#define DS3231_H

#include "driver/i2c_types.h"
#include "esp_err.h"
#include <stdint.h>

#define DS3231_ADDRESS 0x68

typedef struct ds3231_context_t *ds3231_handle_t;

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t dow;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} ds3231_time_t;


/**
 * @brief Initialize the DS3231 RTC and add it to the I2C master bus.
 *
 * @param bus_handle I2C master bus handle.
 * @param ret_handle Pointer to the RTC handle to be initialized.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_init(i2c_master_bus_handle_t bus_handle,
					  ds3231_handle_t *ret_handle);

/**
 * @brief Remove the DS3231 device from the I2C bus and free resources.
 *
 * @param handle RTC handle.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_deinit(ds3231_handle_t ret_handle);

/**
 * @brief Read the current time and date from the DS3231.
 *
 * @param handle RTC handle.
 * @param[out] time Pointer to a ds3231_time_t struct to store the current time.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_get_time(ds3231_handle_t handle, ds3231_time_t *time);

/**
 * @brief Set the current time and date of the DS3231.
 *
 * @param handle RTC handle.
 * @param[in] time Pointer to a ds3231_time_t struct containing the time to set.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_set_time(ds3231_handle_t handle, const ds3231_time_t *time);

/**
 * @brief Check the Oscillator Stop Flag (OSF) to detect power loss.
 * * @note This indicates if the clock has stopped and the time might be invalid.
 *
 * @param handle RTC handle.
 * @param[out] result True if power was lost, false otherwise.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_get_power_loss(ds3231_handle_t handle, bool *result);


/**
 * @brief Read the temperature from the DS3231 internal sensor.
 *
 * @param handle RTC handle.
 * @param[out] temperature Pointer to store the temperature in °C.
 * @return
 * - ESP_OK: Success
 */
esp_err_t ds3231_read_temperature(ds3231_handle_t handle, float *temperature);

#endif
