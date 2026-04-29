
#ifndef MAIN_SYSTEM_LOGIC_H_
#define MAIN_SYSTEM_LOGIC_H_

#include "common_types.h"
#include "ds3231.h"

/**
 * @brief Handle button events while the system is in RUNNING mode.
 * @param context Pointer to the system context (state, sensor data).
 * @param event_packet Structure containing the GPIO and the detected button event.
 */
void procesar_modo_running(system_context_t *contexto,
						   button_event_t paquete_recibido);

/**
 * @brief Manage menu transitions and setup exit logic.
 * @note This function updates the RTC time when exiting the setup menu.
 * @param context Pointer to the system context.
 * @param event_packet Structure containing the GPIO and the detected button event.
 * @param rtc_handle Handle for the DS3231 to save changes.
 */
void procesar_cambio_paso(system_context_t *contexto,
						  button_event_t paquete_recibido,
						  ds3231_handle_t rtc_handle);

/**
 * @brief Handle user inputs to modify values within the configuration menu.
 * @param context Pointer to the system context.
 * @param event_packet Structure containing the GPIO and the detected button event.
 */
void procesar_modo_setup(system_context_t *contexto,
						 button_event_t paquete_recibido);

/**
 * @brief Calculate the maximum number of days in a given month and year.
 * @note Correctly accounts for leap years in February.
 *  @param month Month (1-12).
 * @param year Year (last two digits).
 * @return uint8_t Total number of days (28-31).
 */
uint8_t obtener_dias_del_mes(uint8_t mes, uint8_t anio);

#endif /* MAIN_SYSTEM_LOGIC_H_ */
