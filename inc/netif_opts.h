#ifndef NETIF_OPTS_H
#define NETIF_OPTS_H

// Include Entry Header in STM32
#include "main.h"
#include "app_uart.h"

// Get Tick (Milisecond)
#define NETIF_GET_TIME_MS()     HAL_GetTick()
#ifndef NETIF_GET_TIME_MS()         
    #define NETIF_GET_TIME_MS()          void()
#endif

// Logger
#define NETIF_LOG(buffer,size) UART3_Transmit(buffer,size)
#ifndef NETIF_LOG(buffer, size)
    #define NETIF_LOG(buffer,size)         void(buffer, size)
#endif

// Max Retry
#define NETIF_MAX_RETRY         3
#ifndef NETIF_MAX_RETRY     
    #define NETIF_MAX_RETRY         3
#endif

// Port 4G INOUT to UART
#define NETIF_4G_INPUT(data,data_size)     Sim3g_Receive(data, data_size)
#ifndef NETIF_4G_INPUT(data,data_size)
    #define NETIF_4G_INPUT(data,datasize)   void(data,data_size)
#endif

#define NETIF_4G_OUTPUT(data,data_size)    Sim3g_Transmit(data, data_size)
#ifndef NETIF_4G_OUTPUT(data,data_size)
    #define NETIF_4G_OUTPUT(data,datasize)  void(data, data_size)
#endif


// Port WIFI-ETHERNET INOUT to UART
#ifndef NETIF_WIFI_ETHERNET_INPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_INPUT(data,datasize)   void(data,data_size)
#endif

#ifndef NETIF_WIFI_ETHERNET_OUTPUT(data,data_size)
    #define NETIF_WIFI_ETHERNET_OUTPUT(data,datasize)  void(data, data_size)
#endif

#endif //NETIF_OPTS_H