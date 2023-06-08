#ifndef NETIF_OPTS_H
#define NETIF_OPTS_H

// Include Entry Header in STM32
#include "main.h"
#include "Hal/uart.h"

// Get Tick (Milisecond)
#define NETIF_GET_TIME_MS()     HAL_GetTick()
#ifndef NETIF_GET_TIME_MS()         
    #define NETIF_GET_TIME_MS()          (void)NULL
#endif

// Max Retry
#define NETIF_MAX_RETRY         3
#ifndef NETIF_MAX_RETRY     
    #define NETIF_MAX_RETRY         3
#endif

/***********************************************4G Module*******************************************/
// Power 4G
//#define NETIF_4G_POWER(enable)		if(enable) { \
//											Power_Signal_High(); \
//									}else { \
//											Power_Signal_Low(); \
//									}
#ifndef NETIF_4G_POWER(enable)
    #define NETIF_4G_POWER(enable)             (void)(enable)
#endif
//#define NETIF_4G_RESET(enable)		if(enable) { \
//											Reset_Signal_High(); \
//									}else { \
//											Reset_Signal_Low(); \
//									}
#ifndef NETIF_4G_RESET(enable)
    #define NETIF_4G_RESET(enable)             (void)(enable)
#endif
#define NETIF_4G_POWER_DURATION		2000		// 300ms
#define NETIF_4G_RESET_DURATION		1000		// 300ms
#define NETIF_4G_DELAY_BETWEEN_RESETANDPWRON	3000 // 2s
#define NETIF_4G_WAIT_FOR_STARTUP_DURATION	10000		// 20s
#define NETIF_4G_WAIT_FOR_SOFTWARE_RESET_DURATION	10000		// 20s
// Port 4G INOUT to UART
//#define NETIF_4G_INPUT_IS_AVAILABLE()            Uart1_Received_Buffer_Available()
#ifndef NETIF_4G_INPUT_IS_AVAILABLE()
    #define NETIF_4G_INPUT_IS_AVAILABLE()             0
#endif

//#define NETIF_4G_INPUT()                         Uart1_Read_Received_Buffer()
#ifndef NETIF_4G_INPUT()
    #define NETIF_4G_INPUT()               0
#endif

//#define NETIF_4G_OUTPUT(data,data_size)          Sim3g_Transmit(data, data_size)
#ifndef NETIF_4G_OUTPUT(data,data_size)
    #define NETIF_4G_OUTPUT(data,datasize)              (void)NULL
#endif


/***********************************************Wifi-LAN Module*******************************************/
// Port WIFI-ETHERNET INOUT to UART
#define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()            UART_receive_available(UART_1)
#ifndef NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()    
    #define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()            0
#endif

#define NETIF_WIFI_ETHERNET_INPUT()                         UART_receive_data(UART_1)
#ifndef NETIF_WIFI_ETHERNET_INPUT() 
    #define NETIF_WIFI_ETHERNET_INPUT()                         0
#endif

#define NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)          UART_send(UART_1, data, data_size)
#ifndef NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_OUTPUT(data,datasize)           (void)NULL
#endif

// Core Buffer to handle response
#define NETIF_CORE_BUFFER_SIZE          2048

// AT Command Buffer to send AT Command
#define NETIF_ATCMD_BUFFER_SIZE             1024
#define NETIF_ATCMD_BUFFER_SIZE_LARGE       2048

// Network Command Timeout
#define NETIF_ATCMD_TIMEOUT					1000		// 1000ms

// Network Apps Retry Interval
#define NETIF_APPS_RETRY_INTERVAL			200		// 200ms

// Network Retry Interval
#define NETIF_MANAGER_RETRY_INTERVAL		3000	// 5000ms

#endif //NETIF_OPTS_H
