#ifndef NETIF_OPTS_H
#define NETIF_OPTS_H

// Include Entry Header in STM32
#include "main.h"
#include "app_uart.h"
#include "app_sim3g.h"

// Get Tick (Milisecond)
#define NETIF_GET_TIME_MS() HAL_GetTick()
#ifndef NETIF_GET_TIME_MS()
#define NETIF_GET_TIME_MS() (void)NULL
#endif

// Max Retry
#define NETIF_MAX_RETRY 3
#ifndef NETIF_MAX_RETRY
#define NETIF_MAX_RETRY 3
#endif

#define NETIF_4G_ENABLE 0
#define NETIF_WIFI_ETHERNET_ENABLE 1
#if (!defined(NETIF_4G_ENABLE) || (defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 0)) && \
	(!defined(NETIF_WIFI_ETHERNET_ENABLE) || (defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 0))
#error "Please defined NETIF_4G_ENABLE = 1 or NETIF_WIFI_ETHERNET_ENABLE = 1"
#endif

/***********************************************4G Module*******************************************/
#define SIMCOM7600
// #define SIMCOM7670
#define NETIF_4G_POWER(enable) \
	if (enable)                \
	{                          \
		Power_Signal_High();   \
	}                          \
	else                       \
	{                          \
		Power_Signal_Low();    \
	}

#define NETIF_4G_RESET(enable) \
	if (enable)                \
	{                          \
		Reset_Signal_High();   \
	}                          \
	else                       \
	{                          \
		Reset_Signal_Low();    \
	}
#define NETIF_4G_POWER_DURATION 2000			  // 300ms
#define NETIF_4G_RESET_DURATION 1000			  // 300ms
#define NETIF_4G_DELAY_BETWEEN_RESETANDPWRON 3000 // 2s
#define NETIF_4G_WAIT_FOR_STARTUP_DURATION 60000  // 90s
#define NETIF_4G_WAIT_FOR_SOFTWARE_RESET_DURATION NETIF_4G_WAIT_FOR_STARTUP_DURATION
// Port 4G INOUT to UART
// #define NETIF_4G_INPUT_IS_AVAILABLE()            UART_receive_available(UART_4)
#ifndef NETIF_4G_INPUT_IS_AVAILABLE()
#define NETIF_4G_INPUT_IS_AVAILABLE() 0
#endif

// #define NETIF_4G_INPUT()                         UART_receive_data(UART_4)
#ifndef NETIF_4G_INPUT()
#define NETIF_4G_INPUT() 0
#endif

// #define NETIF_4G_OUTPUT(data,data_size)          UART_send(UART_4, data, data_size)
#ifndef NETIF_4G_OUTPUT(data, data_size)
#define NETIF_4G_OUTPUT(data, datasize) (void)NULL
#endif

/***********************************************Wifi-LAN Module*******************************************/
#define NETIF_WIFI_ETHERNET_RESET(enable) WIFIIO_reset(enable)
#define NETIF_WIFI_ETHERNET_RESET_DELAY 10000	// miliseconds
#define NETIF_WIFI_ETHERNET_RESET_DURATION 2000 // miliseconds
#define NETIF_WIFI_ETHERNET_RESET_TIMEOUT 10000 // miliseconds

// Port WIFI-ETHERNET INOUT to UART
#define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE() UART4_Read_Available()
#ifndef NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()
#define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE() 0
#endif

#define NETIF_WIFI_ETHERNET_INPUT() UART4_Read_Received_Buffer()
#ifndef NETIF_WIFI_ETHERNET_INPUT()
#define NETIF_WIFI_ETHERNET_INPUT() 0
#endif

#define NETIF_WIFI_ETHERNET_OUTPUT(data, data_size) UART4_transmit(data, data_size)
#ifndef NETIF_WIFI_ETHERNET_OUTPUT(data, data_size)
#define NETIF_WIFI_ETHERNET_OUTPUT(data, datasize) (void)NULL
#endif

// Core Buffer to handle response
#define NETIF_CORE_BUFFER_SIZE 2048

// AT Command Buffer to send AT Command
#define NETIF_ATCMD_BUFFER_SIZE 512
#define NETIF_ATCMD_BUFFER_SIZE_LARGE 1024

// Network Command Timeout
#define NETIF_ATCMD_TIMEOUT 10000 // 10000ms

// Network Apps Retry Interval
#define NETIF_APPS_RETRY_INTERVAL 200 // 200ms

// Network Retry Interval
#define NETIF_MANAGER_RETRY_INTERVAL 5000 // 5000ms

#endif // NETIF_OPTS_H
