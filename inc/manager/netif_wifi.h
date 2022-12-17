#ifndef NETIF_WIFI_H
#define NETIF_WIFI_H

#include "stdbool.h"
#include "stdint.h"


// Generic Function
bool netif_wifi_init();
bool netif_wifi_run();
bool netif_wifi_deinit();

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
bool netif_wifi_connect_ap(char *ssid, char * password, uint8_t auth_mode);

/**
 * @brief Disconnect from AP
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_wifi_disconnect_ap();

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
bool netif_wifi_is_connected();

/**
 * @brief Start SmartConfig 
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_wifi_start_smartconfig();

/**
 * @brief Stop Smartconfig
 * 
 * @return true  If OK
 * @return false if failed or timeout
 */
bool netif_wifi_stop_smartconfig();

/**
 * @brief Enable Auto Reconnect to SoftAP
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_wifi_enable_auto_reconnect();

/**
 * @brief Get Ip Address of Wifi Station
 * 
 * @param ip Pointer to Ip String Buffer
 * @param ip_max_size Ip String Buffer Max Size
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_wifi_get_ip(char *ip , size_t ip_max_size){

}

/**
 * @brief Get Mac Address of Wifi Station
 * 
 * @param mac Pointer to Mac String Buffer
 * @param mac_max_size Mac String Buffer Max Size
 * @return true 
 * @return false 
 */
bool netif_wifi_get_mac(char *mac , size_t mac_max_size);

#endif // NETIF_WIFI_H