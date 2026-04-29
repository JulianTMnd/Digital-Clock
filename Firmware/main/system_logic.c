

#include "system_logic.h"
#include "ds3231.h"

uint8_t obtener_dias_del_mes(uint8_t mes, uint8_t anio) {
	uint8_t dias[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0)) {
		dias[2] = 29;
	}

	return dias[mes];
}

void procesar_modo_running(system_context_t *contexto,
						   button_event_t paquete_recibido) {

	if (paquete_recibido.pin != GPIO_NUM_3)
		return;

	if (paquete_recibido.event != LONG_PULSE)
		return;

	contexto->current_mode = MODE_SETUP;
	contexto->button_active = true;
}

void procesar_cambio_paso(system_context_t *contexto,
						  button_event_t paquete_recibido, ds3231_handle_t handle) {

	if (paquete_recibido.pin != GPIO_NUM_3)
		return;

	if (paquete_recibido.event != SHORT_PULSE)
		return;

	contexto->button_active = true;

	switch (contexto->current_step) {
	case SETUP_HOURS:
		contexto->current_step = SETUP_MINUTES;
		break;
	case SETUP_MINUTES:
		contexto->current_step = SETUP_YEAR;
		break;
	case SETUP_YEAR:
		contexto->current_step = SETUP_MONTH;
		break;
	case SETUP_MONTH:
		contexto->current_step = SETUP_DAY;
		if (contexto->current_time.date >
			obtener_dias_del_mes(contexto->current_time.month,
								 contexto->current_time.year)) {
			contexto->current_time.date = obtener_dias_del_mes(
				contexto->current_time.month, contexto->current_time.year);
		}

		break;
	case SETUP_DAY:
		contexto->current_step = SETUP_HOURS;
		contexto->current_mode = MODE_RUNNING;

		ds3231_time_t user_time = {
			.hours = contexto->current_time.hours,
			.minutes = contexto->current_time.minutes,
			.seconds = 0,
			.dow = 1,
			.date = contexto->current_time.date,
			.month = contexto->current_time.month,
			.year = contexto->current_time.year,
		};

		ds3231_set_time(handle, &user_time);

		break;
	default:
		break;
	}
}

void procesar_modo_setup(system_context_t *contexto,
						 button_event_t paquete_recibido) {

	if (paquete_recibido.pin != GPIO_NUM_4) {
		return;
	}

	if (paquete_recibido.event != HOLDING &&
		paquete_recibido.event != SHORT_PULSE) {
		return;
	}

	contexto->button_active = true;

	switch (contexto->current_step) {
	case SETUP_HOURS:
		contexto->current_time.hours++;
		contexto->current_time.hours %= 24;
		break;
	case SETUP_MINUTES:
		contexto->current_time.minutes++;
		contexto->current_time.minutes %= 60;
		break;
	case SETUP_YEAR:
		contexto->current_time.year++;
		contexto->current_time.year %= 100;
		break;
	case SETUP_MONTH:
		contexto->current_time.month++;
		contexto->current_time.month = contexto->current_time.month > 12
										   ? 1
										   : contexto->current_time.month;
		break;
	case SETUP_DAY:
		contexto->current_time.date++;
		contexto->current_time.date =
			contexto->current_time.date >
					obtener_dias_del_mes(contexto->current_time.month,
										 contexto->current_time.year)
				? 1
				: contexto->current_time.date;
		break;
	default:
		break;
	}
}
