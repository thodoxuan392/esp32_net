#ifndef NETIF_WIFI_H
#define NETIF_WIFI_H

#include "stdbool.h"
#include "stdint.h"
#include "netif_def.h"

#define SSID_LEN        64
#define PASSWORD_LEN    128
#define IP_ADDR_LEN     20
#define MAC_ADDR_LEN    20


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
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_run();

/**
 * @brief Deinitialize Wifi Stack
 * 
 * @return true if OK
 * @return false if failed
 */
netif_status_t netif_wifi_deinit();

// Specific Function
/**
 * @brief Connect to SoftAP using SSID and Password
 * 
 * @param ssid SSID of SoftAP
 * @param password Password of SoftAP
 * @param auth_mode Authentication Mode of SoftAP
 * @return true if connected
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_connect_ap(char *ssid, char * password);

/**
 * @brief Disconnect from AP
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_disconnect_ap();

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_is_connected(bool *connected);

/**
 * @brief Start SmartConfig 
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_start_smartconfig();

/**
 * @brief Stop Smartconfig
 * 
 * @return true  If OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_stop_smartconfig();

/**
 * @brief Get Ip Address of Wifi Station
 * 
 * @param ip Pointer to Ip String Buffer
 * @param ip_max_size Ip String Buffer Max Size
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_get_ip(char *ip , size_t ip_max_size){

}

/**
 * @brief Get Mac Address of Wifi Station
 * 
 * @param mac Pointer to Mac String Buffer
 * @param mac_max_size Mac String Buffer Max Size
 * @return true 
 * @return false 
 */
netif_status_t netif_wifi_get_mac(char *mac , size_t mac_max_size);

#endif // NETIF_WIFI_H