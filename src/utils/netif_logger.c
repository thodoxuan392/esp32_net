/*
 * esp32_net_logger.c
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */
#include "stdint.h"
#include "stdarg.h"
#include "utils/netif_logger.h"
#include "netif_opts.h"

static uint8_t log_buffer[NETIF_MAX_LOG_BUFFER];
static uint8_t message_buffer[NETIF_MAX_LOG_BUFFER];

static const char * color_str[NETIF_LOG_ALL] = {
		[NETIF_LOG_OFF] =  "\x1b[0m",
		[NETIF_LOG_ERROR] = "\x1b[31m",		// RED
		[NETIF_LOG_INFO] = "\x1b[32m",		// Green
		[NETIF_LOG_WARN] =  "\x1b[33m",		// Yellow
		[NETIF_LOG_TRACE] = "\x1b[34m",		// Blue
		[NETIF_LOG_FATAL]  = "\x1b[35m",	// Magenta
		[NETIF_LOG_DEBUG] = "\x1b[36m",		//Cyan
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

static netif_log_level_t level = NETIF_LOG_LEVEL;

static char * level_to_color(netif_log_level_t level){
	return color_str[level];
}

static char * level_to_str(netif_log_level_t level){
    return level_str[level];
}


void netif_log_init(netif_log_level_t level){

}


void netif_log_log(netif_log_level_t _level, const char *file, int line, const char *fmt, ...){
    va_list args;
    if(_level > level){
        return;
    }
    va_start(args, fmt);
    va_end(args);
    vsnprintf(message_buffer , NETIF_MAX_LOG_BUFFER , fmt, args);
    int size = snprintf(log_buffer , NETIF_MAX_LOG_BUFFER, "%s%d [%s] %s:%d: %s\r\n%s" ,level_to_color(_level), NETIF_GET_TIME_MS() , level_to_str(_level) , file, line, message_buffer,level_to_color(NETIF_LOG_OFF));
    NETIF_LOG(log_buffer , size);
}



