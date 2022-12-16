/*
 * esp32_net_logger.c
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */


#include "esp32_net_logger.h"
#include "stdint.h"
#include "stdarg.h"

static uint8_t log_buffer[ESP_MAX_LOG_BUFFER];
static uint8_t message_buffer[ESP_MAX_LOG_BUFFER];



static const char * color_str[] = {
		"\x1B[0m",
		"\x1B[31m",
		"\x1B[32m",
		"\x1B[33m",
		"\x1B[34m",
		"\x1B[35m",
		"\x1B[36m",
		"\x1B[37m"
};

static const char * level_str[] = {
		"OFF",
		"FATAL",
		"ERROR",
		"WARN",
		"INFO",
		"DEBUG",
		"TRACE",
		"ALL"
};

static char * level_to_color(esp32net_log_level_t level){
	return color_str[level];
}

static char * level_to_str(esp32net_log_level_t level){
    return level_str[level];
}


void ESP32NET_log_log(esp32net_log_level_t _level, const char *file, int line, const char *fmt, ...){
    va_list args;
    if(_level > ESP32NET_LOG_LEVEL){
        return;
    }
    // Time LOG_LEVEL FILE:LINE: [Message]
    va_start(args, fmt);
    va_end(args);
    vsnprintf(message_buffer , ESP_MAX_LOG_BUFFER , fmt, args);
    int size = snprintf(log_buffer , ESP_MAX_LOG_BUFFER, "%s%d [%s] %s:%d: %s\r\n%s" ,level_to_color(_level), HAL_GetTick() , level_to_str(_level) , file, line, message_buffer,level_to_color(ESP32NET_LOG_OFF));
    ESP32NET_LOG(log_buffer , size);
}
