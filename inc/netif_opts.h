#ifndef NETIF_OPTS_H
#define NETIF_OPTS_H

// Include Entry Header in STM32
#include "main.h"
#include "app_uart.h"

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
#define NETIF_4G_INPUT(data,data_size)     Sim3g_Receive(data, data_size)
#ifndef NETIF_4G_INPUT(data,data_size)
    #define NETIF_4G_INPUT(data,datasize)               (void)NULL
#endif

#define NETIF_4G_OUTPUT(data,data_size)    Sim3g_Transmit(data, data_size)
#ifndef NETIF_4G_OUTPUT(data,data_size)
    #define NETIF_4G_OUTPUT(data,datasize)              (void)NULL
#endif


// Port WIFI-ETHERNET INOUT to UART
#ifndef NETIF_WIFI_ETHERNET_INPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_INPUT(data,datasize)    (void)NULL
#endif

#ifndef NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_OUTPUT(data,datasize)   (void)NULL
#endif

#define NETIF_CORE_BUFFER_SIZE          2048

#endif //NETIF_OPTS_H