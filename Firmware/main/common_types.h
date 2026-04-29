
#ifndef MAIN_COMMON_TYPES_H_
#define MAIN_COMMON_TYPES_H_

#include "esp_err.h"
#include "soc/gpio_num.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Views available for the display.
 */
typedef enum {
    SHOW_TIME,  /**< Display current time and date */
    SHOW_TEMP  /**< Display temperature and humidity */
} display_view_e;


/**
 * @brief Operating modes of the system.
 */
typedef enum {
    MODE_RUNNING, /**< Normal operation mode */
    MODE_SETUP    /**< Configuration/Clock setting mode */
} system_mode_e;

/**
 * @brief Steps for the clock setup wizard.
 */
typedef enum {
	SETUP_HOURS,
	SETUP_MINUTES,
	SETUP_YEAR,
	SETUP_MONTH,
	SETUP_DAY
} setup_step_e;

/**
 * @brief Button events for the debouncing and gesture state machine.
 */
typedef enum {
    IDLE,           /**< No activity */
    DEBOUNCING,     /**< Waiting for signal to stabilize */
    SHORT_PULSE,    /**< Button pressed and released quickly */
    LONG_PULSE,     /**< Button held for a specific duration */
    HOLDING,        /**< Button is currently being held down */
    WAIT_RELEASE,   /**< Waiting for button release after an event */
} button_event_e;


/**
 * @brief Application-level time structure.
 * @note This is independent of the RTC driver to ensure abstraction.
 */
typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t dow;    /**< Day of the week */
	uint8_t date;   /**< Day of the month */
	uint8_t month;
	uint8_t year;
} app_time_t;

/**
 * @brief Global system context to store current state and sensor data.
 */
typedef struct {
    system_mode_e current_mode;   /**< Current operating mode */
    setup_step_e current_step;    /**< Current step if in SETUP mode */
    display_view_e current_view;  /**< Currently selected display view */
    app_time_t current_time;      /**< Last read time data */
    float temperature;            /**< Last read temperature value */
    bool button_active;           /**< Flag indicating if a button is being pressed */
    esp_err_t err_rtc;            /**< Last error status from RTC */
    esp_err_t err_aht;            /**< Last error status from AHT sensor */
} system_context_t;

/**
 * @brief Structure to pass button events through queues
 */
typedef struct {
    gpio_num_t pin;          /**< GPIO pin that triggered the event */
    button_event_e event;    /**< Type of event detected */
} button_event_t;


#endif /* MAIN_COMMON_TYPES_H_ */
