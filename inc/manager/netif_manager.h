#ifndef NETIF_MANAGER_H
#define NETIF_MANAGER_H

#include "stdbool.h"
#include "netif_def.h"


typedef enum{
    NETIF_MANAGER_DISCONNECTED_MODE,
    NETIF_MANAGER_4G_MODE,
    NETIF_MANAGER_WIFI_ETHERNET_MODE,
}netif_manager_state_t;

/**
 * @brief Initialize Net Manager
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_init();

/**
 * @brief Run Net Manager in Supper Loop
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_run();

/**
 * @brief Deinitialize Net Manager
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_deinit();

/**
 * @brief Check if is running 4G Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_4g_mode(bool *connected);


/**
 * @brief Check if is running Wifi/Ethernet Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_wifi_ethernet_mode(bool *connected);

#endif //NETIF_MANAGER_H