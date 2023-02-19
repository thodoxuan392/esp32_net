#ifndef NETIF_MANAGER_H
#define NETIF_MANAGER_H

#include "stdbool.h"
#include "netif_def.h"


typedef enum{
    NETIF_MANAGER_DISCONNECTED_MODE,
    NETIF_MANAGER_4G_MODE,
	NETIF_MANAGER_ETHERNET_MODE,
    NETIF_MANAGER_WIFI_MODE,
}netif_manager_mode_t;

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
 * @brief Check Internet Connection
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_connect_to_internet(bool *connected);

/**
 * @brief Check if is running 4G Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_4g_mode(bool *connected);


/**
 * @brief Check if is running Ethernet Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_ethernet_mode(bool *connected);


/**
 * @brief Check if is running Wifi Mode
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_wifi_mode(bool *connected);


/**
 * @brief Get Current Mode of Network Manager
 *
 * @return netif_manager_mode_t network mode
 */
netif_manager_mode_t netif_manager_get_mode();

#endif //NETIF_MANAGER_H
