/*
 * NETIF_logger.h
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */

#ifndef NETIF_LOGGER_H_
#define NETIF_LOGGER_H_

#define NETIF_MAX_LOG_BUFFER 1024

typedef enum{
    NETIF_LOG_OFF,
	NETIF_LOG_FATAL,
	NETIF_LOG_ERROR,
	NETIF_LOG_WARN,
	NETIF_LOG_INFO,
	NETIF_LOG_DEBUG,
	NETIF_LOG_TRACE,
	NETIF_LOG_ALL
}netif_log_level_t;


#define netif_log_trace(...) netif_log_log(NETIF_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define netif_log_debug(...) netif_log_log(NETIF_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define netif_log_info(...)  netif_log_log(NETIF_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define netif_log_warn(...)  netif_log_log(NETIF_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define netif_log_error(...) netif_log_log(NETIF_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define netif_log_fatal(...) netif_log_log(NETIF_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void netif_log_init(netif_log_level_t level);
void netif_log_log(netif_log_level_t level, const char *file, int line, const char *fmt, ...);


#endif /* NETIF_LOGGER_H_ */
