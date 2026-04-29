#include "ds3231.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct ds3231_context_t {
  i2c_master_dev_handle_t i2c_dev;
} ds3231_context_t;

typedef enum {
  SECONDS_ADDRESS = 0x00,
  MINUTES_ADDRESS = 0x01,
  HOURS_ADDRESS = 0x02,
  DOW_ADDRESS = 0x03,
  DATE_ADDRESS = 0x04,
  MONTH_ADDRESS = 0x05,
  YEAR_ADDRESS = 0x06,
  ALARM1_SECONDS_ADDRESS = 0x07,
  ALARM1_MINUTES_ADDRESS = 0x08,
  ALARM1_HOUR_ADDRESS = 0x09,
  ALARM1_DAY_DATE_ADDRESS = 0x0A,
  ALARM2_MINUTES_ADDRESS = 0x0B,
  ALARM2_HOUR_ADDRESS = 0x0C,
  ALARM2_DAY_DATE_ADDRESS = 0x0D,
  CONTROL_REGISTER_ADDRESS = 0x0E,
  STATUS_REGISTER_ADDRESS = 0x0F,
  AGING_OFFSET_ADDRESS = 0x10,
  TEMPERATURE_REGISTER_UPPER_ADDRESS = 0x11,
  TEMPERATURE_REGISTER_LOWER_ADDRESS = 0x12,
} DS3231_address_e;

static int dec_to_bcd(int dec) {
  dec = ((dec / 10) << 4) + (dec % 10);
  return dec;
}

static int bcd_to_dec(uint8_t bcd) {
  return ((bcd >> 4) * 10) + ((bcd & 0x0F));
}

static esp_err_t ds3231_read_registers(ds3231_handle_t handle,
                                       uint8_t read_address, uint8_t *data,
                                       size_t len) {

  if (handle == NULL || data == NULL || len == 0)
    return ESP_ERR_INVALID_ARG;

  ds3231_context_t *sensor = (ds3231_context_t *)handle;

  if (sensor->i2c_dev == NULL)
    return ESP_ERR_INVALID_STATE;

  esp_err_t err = i2c_master_transmit_receive(sensor->i2c_dev, &read_address, 1,
                                              data, len, pdMS_TO_TICKS(100));

  return err;
}

static esp_err_t ds3231_write_reg(ds3231_handle_t handle, uint8_t write_address,
                                  uint8_t *data, size_t len) {


  ds3231_context_t *sensor = (ds3231_context_t *)handle;
  if (!sensor || !sensor->i2c_dev)
    return ESP_ERR_INVALID_STATE;

  uint8_t buffer[1 + len];


  buffer[0] = write_address;
  memcpy(&buffer[1], data, len);

  esp_err_t err = i2c_master_transmit(sensor->i2c_dev, buffer, len + 1, 100);


  return err;
}

esp_err_t ds3231_init(i2c_master_bus_handle_t bus_handle,
                      ds3231_handle_t *ret_handle) {

  if (bus_handle == NULL || ret_handle == NULL)
    return ESP_ERR_INVALID_ARG;

  ds3231_context_t *sensor =
      (ds3231_context_t *)calloc(1, sizeof(ds3231_context_t));
  if (sensor == NULL) {
    return ESP_ERR_NO_MEM;
  }

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = DS3231_ADDRESS,
      .scl_speed_hz = 400000,
  };


  esp_err_t err =
      i2c_master_bus_add_device(bus_handle, &dev_cfg, &sensor->i2c_dev);


  if (err != ESP_OK) {
    free(sensor);
    return err;
  }

  *ret_handle = sensor;

  return ESP_OK;
}

esp_err_t ds3231_deinit(ds3231_handle_t ret_handle) {

  if (ret_handle == NULL)
    return ESP_ERR_INVALID_ARG;

  ds3231_context_t *sensor = (ds3231_context_t *)ret_handle;

  esp_err_t err = i2c_master_bus_rm_device(sensor->i2c_dev);

  free(ret_handle);

  return err;
}

esp_err_t ds3231_get_time(ds3231_handle_t handle, ds3231_time_t *time) {

  if (handle == NULL || time == NULL)
    return ESP_ERR_INVALID_ARG;

  uint8_t initial_word_address = SECONDS_ADDRESS;
  uint8_t read_buffer[7];

  esp_err_t err = ds3231_read_registers(handle, initial_word_address,
                                        read_buffer, sizeof(read_buffer));
  if (err != ESP_OK)
    return err;

  time->seconds = bcd_to_dec(read_buffer[SECONDS_ADDRESS]);
  time->minutes = bcd_to_dec(read_buffer[MINUTES_ADDRESS]);
  time->hours = bcd_to_dec(read_buffer[HOURS_ADDRESS] & 0x3F);
  time->dow = bcd_to_dec(read_buffer[DOW_ADDRESS]);
  time->date = bcd_to_dec(read_buffer[DATE_ADDRESS]);
  time->month = bcd_to_dec(read_buffer[MONTH_ADDRESS] & 0x7F);
  time->year = bcd_to_dec(read_buffer[YEAR_ADDRESS]);

  return ESP_OK;
}

esp_err_t ds3231_set_time(ds3231_handle_t handle, const ds3231_time_t *time) {

  if (handle == NULL || time == NULL)
    return ESP_ERR_INVALID_ARG;

  uint8_t write_buffer[7];
  write_buffer[SECONDS_ADDRESS] = dec_to_bcd(time->seconds);
  write_buffer[MINUTES_ADDRESS] = dec_to_bcd(time->minutes);
  write_buffer[HOURS_ADDRESS] = dec_to_bcd(time->hours);
  write_buffer[DOW_ADDRESS] = dec_to_bcd(time->dow);
  write_buffer[DATE_ADDRESS] = dec_to_bcd(time->date);
  write_buffer[MONTH_ADDRESS] = dec_to_bcd(time->month);
  write_buffer[YEAR_ADDRESS] = dec_to_bcd(time->year);

  esp_err_t err = ds3231_write_reg(handle, SECONDS_ADDRESS, write_buffer,
                                   sizeof(write_buffer));
  return err;
}

esp_err_t ds3231_get_power_loss(ds3231_handle_t handle, bool *result) {

  if (handle == NULL || result == NULL)
    return ESP_ERR_INVALID_ARG;

  uint8_t buffer;

  esp_err_t err = ds3231_read_registers(handle, STATUS_REGISTER_ADDRESS,
                                        &buffer, sizeof(buffer));
  if (err != ESP_OK)
    return err;

  *result = (buffer >> 7) & 1;

  if (*result) {
    buffer = buffer & 0x7F;
    err = ds3231_write_reg(handle, STATUS_REGISTER_ADDRESS, &buffer,
                           sizeof(buffer));
  }

  return err;
}

esp_err_t ds3231_read_temperature(ds3231_handle_t handle, float *temperature) {

  if (handle == NULL || temperature == NULL)
    return ESP_ERR_INVALID_ARG;

  uint8_t register_value;
  bool busy_flag = true;
  int8_t time_out = 100;

  esp_err_t err = ds3231_read_registers(handle, CONTROL_REGISTER_ADDRESS,
                                        &register_value, 1);
  if (err != ESP_OK)
    return err;


  register_value = (register_value | 0x20);
  err = ds3231_write_reg(handle, CONTROL_REGISTER_ADDRESS, &register_value,
                         sizeof(register_value));

  if (err != ESP_OK)
    return err;


  while (busy_flag && time_out > 0) {
    err = ds3231_read_registers(handle, STATUS_REGISTER_ADDRESS,
                                &register_value, 1);
    if (err != ESP_OK)
      return err;
    busy_flag = (register_value >> 2) & 1;
    vTaskDelay(pdMS_TO_TICKS(5));
    time_out--;
  }

  if (time_out <= 0) {
    return ESP_ERR_TIMEOUT;
  }


  uint8_t read_buffer[2];
  err = ds3231_read_registers(handle, TEMPERATURE_REGISTER_UPPER_ADDRESS,
                              read_buffer, sizeof(read_buffer));
  if (err != ESP_OK)
    return err;



  int16_t temp = ((int16_t)read_buffer[0] << 8) | read_buffer[1];
  temp >>= 6;

  *temperature = temp * 0.25f;

  return err;
}
