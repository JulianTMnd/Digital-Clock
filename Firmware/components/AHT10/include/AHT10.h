
#ifndef _AHT10_H_
#define _AHT10_H_

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct aht10_context_t *aht10_handle_t;

/**
 * @brief Initialize the AHT10 sensor and add it to the I2C master bus.
 *
 * @param bus_handle I2C master bus handle.
 * @param ret_handle Pointer to the sensor handle to be initialized.
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_NO_MEM: Memory allocation failed
 */
esp_err_t aht10_init(i2c_master_bus_handle_t bus_handle,
                     aht10_handle_t *ret_handle);

/**
 * @brief Remove the AHT10 device from the I2C bus and delete the handle.
 *
 * @param handle Sensor handle.
 * @return
 * - ESP_OK: Success
 */
esp_err_t aht10_deinit(aht10_handle_t handle);

/**
 * @brief Read temperature and humidity data from the AHT10 sensor.
 *
 * @param handle Sensor handle.
 * @param[out] temperature Pointer to store the temperature value in °C.
 * @param[out] humidity Pointer to store the humidity value in %.
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_TIMEOUT: I2C timeout
 */

esp_err_t aht10_read(aht10_handle_t handle, float *temperature,
                     float *humidity);

/**
 * @brief Perform a soft reset of the sensor.
 * * This restarts the sensor logic without requiring a physical power cycle.
 *
 * @param handle Sensor handle.
 * @return
 * - ESP_OK: Success
 */
esp_err_t aht10_soft_reset(aht10_handle_t handle);

#endif
