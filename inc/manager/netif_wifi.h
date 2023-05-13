#ifndef NETIF_WIFI_H
#define NETIF_WIFI_H

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "netif_def.h"

#define SSID_LEN            64
#define PASSWORD_LEN        128
#define IP_ADDR_LEN         20
#define MAC_ADDR_LEN        20
#define RETRY_CONNECT_AP    5
#define RETRY_INTERVAL      5000

enum {
	NETIF_WIFI_NO_ANY_CONNECTION_BEFORE = 0,
	NETIF_WIFI_HAS_CONNECT_NO_IP = 1,
	NETIF_WIFI_HAS_CONNECT_IP = 2,
	NETIF_WIFI_IS_RECONNECTING = 3,
	NETIF_WIFI_IS_DISCONNECTED = 4
};


// Generic Function

/**
 * @brief Initialize 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_wifi_init();

/**
 * @brief Run Wifi Stack in Supper loop, handle event, ...
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_run();

/**
 * @brief Deinitialize Wifi Stack
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_deinit();

// Specific Function
/**
 * @brief Connect to SoftAP using SSID and Password
 * 
 * @param ssid SSID of SoftAP
 * @param password Password of SoftAP
 * @param auth_mode Authentication Mode of SoftAP
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_connect_ap(char *ssid, char * password);

/**
 * @brief Disconnect from AP
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_disconnect_ap();

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_is_connected(bool *connected);

/**
 * @brief Reconnect to last wifi
 *
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_reconnect();

/**
 * @brief Start SmartConfig 
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_start_smartconfig();

/**
 * @brief Stop Smartconfig
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_wifi_stop_smartconfig();



#endif // NETIF_WIFI_H
