/*
 * esp32_net_logger.h
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */

#ifndef ESP32_NET_UTILS_ESP32_NET_LOGGER_H_
#define ESP32_NET_UTILS_ESP32_NET_LOGGER_H_

#define ESP_MAX_LOG_BUFFER 1024

#include "app_debug.h"

#define ESP32NET_LOG_LEVEL	ESP32NET_LOG_DEBUG
#define ESP32NET_LOG APPDEBUG_log_raw

#ifndef ESP32NET_LOG
	#define ESP32NET_LOG	(void) //Ignore Log
#endif

typedef enum{
    ESP32NET_LOG_OFF,
	ESP32NET_LOG_FATAL,
	ESP32NET_LOG_ERROR,
	ESP32NET_LOG_WARN,
	ESP32NET_LOG_INFO,
	ESP32NET_LOG_DEBUG,
	ESP32NET_LOG_TRACE,
	ESP32NET_LOG_ALL
}esp32net_log_level_t;


#define ESP32NET_log_trace(...) ESP32NET_log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define ESP32NET_log_debug(...) ESP32NET_log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define ESP32NET_log_info(...)  ESP32NET_log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define ESP32NET_log_warn(...)  ESP32NET_log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define ESP32NET_log_error(...) ESP32NET_log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define ESP32NET_log_fatal(...) ESP32NET_log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void ESP32NET_log_log(esp32net_log_level_t level, const char *file, int line, const char *fmt, ...);


#endif /* ESP32_NET_UTILS_ESP32_NET_LOGGER_H_ */
