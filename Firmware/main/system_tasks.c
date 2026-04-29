
#include "system_tasks.h"
#include "ds3231.h"
#include "esp_log.h"
#include "segmentos.h"
#include "system_logic.h"

const char *AHT10_TAG = "AHT10";
const char *DS3231_TAG = "DS3231";

void dibujar_pantalla(uint8_t digito_0, uint8_t digito_1, uint8_t digito_2,
					  uint8_t digito_3, bool punto_central,
					  segment7_handle_t segment_handle) {
	
	segment7_set_raw(segment_handle, 0, digito_0, 0);
	segment7_set_raw(segment_handle, 1, digito_1, 0);
	segment7_set_raw(segment_handle, 2, digito_2, punto_central);
	segment7_set_raw(segment_handle, 3, digito_3, 0);
	
}

void display_task(void *pvParameters) {

	display_task_params_t *parameters = (display_task_params_t *)pvParameters;

	system_context_t contexto_local = {0};
	uint8_t contador_blink = 0;
	bool mostrar_digitos = true;
	uint16_t contador_rotacion = 0;

	for (;;) {

		system_context_t nuevo_contexto;
		if (xQueueReceive(parameters->display_queue, &nuevo_contexto, 0)) {
			nuevo_contexto.current_view = contexto_local.current_view;
			contexto_local = nuevo_contexto;
		}

		// logica de temporizador
		if (contexto_local.current_mode == MODE_SETUP) {

			if (contexto_local.button_active == false) {
				contador_blink++;
				if (contador_blink >= 25) {
					contador_blink = 0;
					mostrar_digitos = !mostrar_digitos;
				}
			} else {
				contador_blink = 0;
				mostrar_digitos = true;
			}
		}


		if (contexto_local.current_mode == MODE_SETUP) {
			switch (contexto_local.current_step) {
			case SETUP_HOURS:
				dibujar_pantalla(
					mostrar_digitos
						? (contexto_local.current_time.hours / 10 % 10)
						: SEG_CODE_BLANK,
					mostrar_digitos ? (contexto_local.current_time.hours % 10)
									: SEG_CODE_BLANK,
					contexto_local.current_time.minutes / 10 % 10,
					(contexto_local.current_time.minutes % 10), 1,
					parameters->segments);
				break;

			case SETUP_MINUTES:
				dibujar_pantalla(
					(contexto_local.current_time.hours / 10 % 10),
					(contexto_local.current_time.hours % 10),
					mostrar_digitos
						? (contexto_local.current_time.minutes / 10 % 10)
						: SEG_CODE_BLANK,
					mostrar_digitos ? (contexto_local.current_time.minutes % 10)
									: SEG_CODE_BLANK,
					1, parameters->segments);
				break;

			case SETUP_YEAR:
				dibujar_pantalla(
					2, 0,
					mostrar_digitos
						? (contexto_local.current_time.year / 10 % 10)
						: SEG_CODE_BLANK,
					mostrar_digitos ? (contexto_local.current_time.year % 10)
									: SEG_CODE_BLANK,
					0, parameters->segments);
				break;
			case SETUP_MONTH:
				dibujar_pantalla(
					mostrar_digitos
						? (contexto_local.current_time.month / 10 % 10)
						: SEG_CODE_BLANK,
					mostrar_digitos ? (contexto_local.current_time.month % 10)
									: SEG_CODE_BLANK,
					contexto_local.current_time.date / 10 % 10,
					(contexto_local.current_time.date % 10), 1,
					parameters->segments);
				break;
			case SETUP_DAY:
				dibujar_pantalla(
					(contexto_local.current_time.month / 10 % 10),
					(contexto_local.current_time.month % 10),
					mostrar_digitos
						? (contexto_local.current_time.date / 10 % 10)
						: SEG_CODE_BLANK,
					mostrar_digitos ? (contexto_local.current_time.date % 10)
									: SEG_CODE_BLANK,
					1, parameters->segments);
				break;
			}

		} else {

			contador_rotacion++;

			switch (contexto_local.current_view) {

			case SHOW_TIME:
				if (contador_rotacion > 1000) {
					contexto_local.current_view = SHOW_TEMP;
					contador_rotacion = 0;
				}
				if (contexto_local.err_rtc != ESP_OK) {

					dibujar_pantalla(SEG_CODE_E, SEG_CODE_R, SEG_CODE_R, 1, 0,
									 parameters->segments);
				} else {
					bool dots;
					if (contexto_local.current_time.seconds % 2) {
						dots = true;
					} else {
						dots = false;
					}
					dibujar_pantalla(
						contexto_local.current_time.hours >= 10
							? (contexto_local.current_time.hours / 10 % 10)
							: SEG_CODE_BLANK,
						(contexto_local.current_time.hours % 10),
						(contexto_local.current_time.minutes / 10 % 10),
						(contexto_local.current_time.minutes % 10), dots,
						parameters->segments);
				}

				break;

			case SHOW_TEMP:

				if (contador_rotacion > 300) {
					contexto_local.current_view = SHOW_TIME;
					contador_rotacion = 0;
				}

				if (contexto_local.err_aht != ESP_OK) {
					// Muestra Err 2 para indicar error en temperatura
					dibujar_pantalla(SEG_CODE_E, SEG_CODE_R, SEG_CODE_R, 2, 0,
									 parameters->segments);
				} else {

					if (contexto_local.temperature < 0) {
						dibujar_pantalla(
							SEG_CODE_DASH,
							((int8_t)contexto_local.temperature * (int8_t)-1) %
								10,
							SEG_CODE_DEGREE, SEG_CODE_C, 0,
							parameters->segments);

					} else {

						dibujar_pantalla(
							(uint8_t)(contexto_local.temperature) / 10 % 10,
							(uint8_t)(contexto_local.temperature) % 10,
							SEG_CODE_DEGREE, SEG_CODE_C, 0,
							parameters->segments);
					}
				}

				break;
			}
		}

		segment7_refresh(parameters->segments);
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void sensor_task(void *pvParameters) {
	sensor_task_params_t *parameters = (sensor_task_params_t *)pvParameters;
	system_context_t contexto = {.current_mode = MODE_RUNNING,
								 .current_step = SETUP_HOURS,
								 .current_view = SHOW_TIME,
								 .current_time = {0},
								 .button_active = false,
								 .err_aht = ESP_OK,
								 .err_rtc = ESP_OK,
								 .temperature = 0};
	for (;;) {

		button_event_t paquete_recibido;
		if (xQueueReceive(parameters->button_queue, &paquete_recibido,
						  pdMS_TO_TICKS(200))) {

			switch (contexto.current_mode) {
			case MODE_RUNNING:
				procesar_modo_running(&contexto, paquete_recibido);

				break;

			case MODE_SETUP:
				procesar_cambio_paso(&contexto, paquete_recibido,
									 parameters->rtc);
				procesar_modo_setup(&contexto, paquete_recibido);
				break;
			}

			xQueueSend(parameters->display_queue, &contexto, 0);

		} else {

			bool send = false;
			static int sensor_count = 0;

			if (contexto.button_active) {
				contexto.button_active = false;
				send = true;
			}

			if (contexto.current_mode == MODE_RUNNING) {

				ds3231_time_t rtc_time;
				esp_err_t err_rtc = ds3231_get_time(parameters->rtc, &rtc_time);
				contexto.err_rtc = err_rtc;
				if (err_rtc != ESP_OK) {
					ESP_LOGE(DS3231_TAG, "ERROR AL LEER RTC");
				} else {
					contexto.current_time.hours = rtc_time.hours;
					contexto.current_time.minutes = rtc_time.minutes;
					contexto.current_time.seconds = rtc_time.seconds;
					contexto.current_time.dow = rtc_time.dow;
					contexto.current_time.date = rtc_time.date;
					contexto.current_time.month = rtc_time.month;
					contexto.current_time.year = rtc_time.year;
				}

				sensor_count++;

				if (sensor_count >= 10) {
					sensor_count = 0;
					float temperatura, humedad;
					esp_err_t err_aht = aht10_read(parameters->temp_sensor,
												   &temperatura, &humedad);
					contexto.err_aht = err_aht;
					if (err_aht != ESP_OK) {
						ESP_LOGE(AHT10_TAG, "ERROR AL LEER AHT10");
					} else {
						contexto.temperature = temperatura;
					}
				}

				send = true;
			}

			if (send) {
				xQueueSend(parameters->display_queue, &contexto, 0);
				send = false;
			}
		}
	}
}


void button_task(void *pvParameters) {

	button_context_t buttons[] = {
		{.state = IDLE, .pin = GPIO_NUM_4, .time_pressed = 0},
		{.state = IDLE, .pin = GPIO_NUM_3, .time_pressed = 0}};

	uint8_t total_buttons = sizeof(buttons) / sizeof(buttons[0]);

	button_task_params_t *parameters = (button_task_params_t *)pvParameters;

	for (;;) {

		for (int button_index = 0; button_index < total_buttons;
			 button_index++) {

			bool is_pressed = (gpio_get_level(buttons[button_index].pin) == 0);

			switch (buttons[button_index].state) {

			case IDLE:
				if (is_pressed) {
					buttons[button_index].state = DEBOUNCING;
				}
				break;

			case DEBOUNCING:
				if (is_pressed) {
					buttons[button_index].time_pressed++;
					if (buttons[button_index].time_pressed >= 3)
						buttons[button_index].state = SHORT_PULSE;
				} else {
					buttons[button_index].time_pressed = 0;
					buttons[button_index].state = WAIT_RELEASE;
				}
				break;

			case SHORT_PULSE:

				if (!is_pressed) {
					button_event_t event = {.event = SHORT_PULSE,
											.pin = buttons[button_index].pin};
					xQueueSend(parameters->button_queue, &event, 0);
					buttons[button_index].time_pressed = 0;
					buttons[button_index].state = WAIT_RELEASE;

				} else {
					buttons[button_index].time_pressed++;
					if (buttons[button_index].time_pressed >= 50) {
						buttons[button_index].state = LONG_PULSE;
					}
				}
				break;

			case LONG_PULSE:
				if (is_pressed) {
					button_event_t event = {.event = LONG_PULSE,
											.pin = buttons[button_index].pin};
					xQueueSend(parameters->button_queue, &event, 0);
					buttons[button_index].state = HOLDING;
					buttons[button_index].time_pressed = 0;
				} else {
					buttons[button_index].time_pressed = 0;
					buttons[button_index].state = WAIT_RELEASE;
				}
				break;

			case HOLDING:
				if (is_pressed) {
					buttons[button_index].time_pressed++;
					if (buttons[button_index].time_pressed > 25) {
						button_event_t event = {
							.event = HOLDING, .pin = buttons[button_index].pin};
						xQueueSend(parameters->button_queue, &event, 0);
						buttons[button_index].time_pressed = 0;
					}
				} else {
					buttons[button_index].state = WAIT_RELEASE;
					buttons[button_index].time_pressed = 0;
				}
				break;
			case WAIT_RELEASE:
				if (!is_pressed) {
					buttons[button_index].time_pressed++;
					if (buttons[button_index].time_pressed > 4) {
						buttons[button_index].state = IDLE;
						buttons[button_index].time_pressed = 0;
					}
				} else {
					buttons[button_index].time_pressed = 0;
				}

				break;

			default:
				break;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
