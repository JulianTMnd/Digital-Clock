/*
 * system_tasks.h
 *
 *  Created on: 1 mar. 2026
 *      Author: Julian
 */

#ifndef MAIN_SYSTEM_TASKS_H_
#define MAIN_SYSTEM_TASKS_H_

#include "AHT10.h"
#include "common_types.h"
#include "ds3231.h"
#include "freertos/idf_additions.h"
#include "segmentos.h"


/**
 * @brief Internal context for button debouncing and state tracking.
 */
typedef struct {
    gpio_num_t pin;           /**< Assigned GPIO pin for the button */
    button_event_e state;     /**< Current state in the debouncing FSM */
    uint8_t time_pressed;     /**< Counter to differentiate between short and long pulses */
} button_context_t;

/**
 * @brief Parameters for the Input/Button monitoring task.
 */
typedef struct {
    QueueHandle_t button_queue; /**< Handle to the queue for dispatching button events */
} button_task_params_t;

/**
 * @brief Parameters for the Display management task.
 */
typedef struct {
    QueueHandle_t display_queue;  /**< Handle to the queue for receiving formatted display data */
    segment7_handle_t segments;   /**< Driver handle for the 7-segment display hardware */
} display_task_params_t;

/**
 * @brief Parameters for the Sensor and Logic task.
 */
typedef struct {
    QueueHandle_t button_queue;   /**< Queue handle to receive user input events */
    QueueHandle_t display_queue;  /**< Queue handle to send processed data to the display */
    ds3231_handle_t rtc;          /**< Handle for the DS3231 Real-Time Clock */
    aht10_handle_t temp_sensor;   /**< Handle for the AHT10 Temperature/Humidity sensor */
} sensor_task_params_t;


/**
 * @brief Display Task: Handles 7-segment multiplexing and visual output.
 * @details Responsible for updating the physical displays based on the
 * current system mode (Running or Setup). It consumes data from the display_queue.
 * * @param pvParameters Pointer to a display_task_params_t structure.
 */
void display_task(void *pvParameters);

/**
 * @brief Sensor Task: The central logic and data acquisition engine.
 * * @details Orchestrates the system flow by processing button events,
 * polling environmental sensors, and calculating the time/date.
 * It acts as a bridge between inputs and visual output.
 * * @param pvParameters Pointer to a sensor_task_params_t structure.
 */
void sensor_task(void *pvParameters);

/**
 * @brief Button Task: Monitors physical inputs and performs debouncing.
 * * @details Scans GPIO inputs, executes the debouncing state machine,
 * and pushes identified event gestures (Short/Long press) into the button_queue.
 * * @param pvParameters Pointer to a button_task_params_t structure.
 */
void button_task(void *pvParameters);

#endif /* MAIN_SYSTEM_TASKS_H_ */
