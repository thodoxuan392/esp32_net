#ifndef NETIF_OPTS_H
#define NETIF_OPTS_H

// Include Entry Header in STM32
#include "main.h"
#include "app_uart.h"

#define NETIF_LOG_LEVEL			UTILS_LOG_ALL

// Get Tick (Milisecond)
#define NETIF_GET_TIME_MS()     HAL_GetTick()
#ifndef NETIF_GET_TIME_MS()         
    #define NETIF_GET_TIME_MS()          (void)NULL
#endif

// Logger
#define NETIF_LOG(buffer,size) UART3_Transmit(buffer,size)
#ifndef NETIF_LOG(buffer, size)
    #define NETIF_LOG(buffer,size)          (void)NULL
#endif

// Max Retry
#define NETIF_MAX_RETRY         3
#ifndef NETIF_MAX_RETRY     
    #define NETIF_MAX_RETRY         3
#endif

// Port 4G INOUT to UART
#ifndef NETIF_4G_INPUT_IS_AVAILABLE()
    #define NETIF_4G_INPUT_IS_AVAILABLE()             0
#endif

#ifndef NETIF_4G_INPUT()
    #define NETIF_4G_INPUT()               0
#endif

#ifndef NETIF_4G_OUTPUT(data,data_size)
    #define NETIF_4G_OUTPUT(data,datasize)              (void)NULL
#endif


// Port WIFI-ETHERNET INOUT to UART
#define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()            Uart1_Received_Buffer_Available()
#ifndef NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()    
    #define NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()            0
#endif

#define NETIF_WIFI_ETHERNET_INPUT()                         Uart1_Read_Received_Buffer()
#ifndef NETIF_WIFI_ETHERNET_INPUT() 
    #define NETIF_WIFI_ETHERNET_INPUT()                         0
#endif

#define NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)          Sim3g_Transmit(data, data_size)
#ifndef NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_OUTPUT(data,datasize)           (void)NULL
#endif

// Core Buffer to handle response
#define NETIF_CORE_BUFFER_SIZE          2048

// AT Command Buffer to send AT Command
#define NETIF_ATCMD_BUFFER_SIZE             1024
#define NETIF_ATCMD_BUFFER_SIZE_LARGE       2048

// Network Apps Retry Interval
#define NETIF_APPS_RETRY_INTERVAL			200	//3s

// Network Retry Interval
#define NETIF_MANAGER_RETRY_INTERVAL		10000	//3s

#endif //NETIF_OPTS_H
