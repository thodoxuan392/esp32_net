#ifndef NETIF_CORE_H
#define NETIF_CORE_H

#include "stdbool.h"
#include "stdint.h"
#include "netif_opts.h"
#include "netif_def.h"

typedef enum{
    NETIF_RESPONSE_OK = 0,
    NETIF_RESPONSE_ERROR,
    NETIF_RESPONSE_INPUT,
    // WifiEthernet Generic
    NETIF_WIFI_ETHERNET_RESPONSE_SEND_OK,
    NETIF_WIFI_ETHERNET_RESPONSE_SEND_FAIL,
    NETIF_WIFI_ETHERNET_RESPONSE_SET_OK,
    NETIF_WIFI_ETHERNET_REPORT_READY,
    NETIF_WIFI_ETHERNET_REPORT_BUSY,
    NETIF_WIFI_ETHERNET_REPORT_FORCE_RESTART,
    // Smart Config
    NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_TYPE,
    NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_INFO,
    NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_CONNECTED_AP,
    // Wifi
    NETIF_WIFI_ETHERNET_REPORT_WIFI_CONNECTED,
    NETIF_WIFI_ETHERNET_REPORT_WIFI_GOT_IP,
    NETIF_WIFI_ETHERNET_REPORT_WIFI_DISCONNECTED,
    // Ethernet
    NETIF_WIFI_ETHERNET_REPORT_ETH_CONNECTED,
    NETIF_WIFI_ETHERNET_REPORT_ETH_GOT_IP,
    NETIF_WIFI_ETHERNET_REPORT_ETH_DISCONNECTED,
    // Wifi_Ethernet MQTT
    NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED,
    NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED,
    NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK,
    NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK,
    NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL,

	//UNKNOWN
	NETIF_RESPONSE_UNKNOWN
}netif_core_response_t;

// Generic Function
/**
 * @brief Initialize Network Interface Core Stack
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_core_init();

/**
 * @brief Run Network Interface Core Stack in Supper Loop
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_core_run();

/**
 * @brief Deinitialize Network Interface Core Stack
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_core_deinit();

/**
 * @brief Check ATCMD is have response match with atcommand table
 * 
 * @param response Pointer to response
 * @return true if have response
 * @return false if not
 */
bool netif_core_atcmd_is_responded(netif_core_response_t* response);

/**
 * @brief Get Data Before the AT Reponse
 * 
 * @param data Pointer to data*, Get buffer Pointer of Core Buffer
 * @param data_size Get Size
 * @return true if OK
 * @return false if failed
 */
bool netif_core_atcmd_get_data_before(uint8_t **data, size_t * data_size);


/**
 * @brief Get Data after the AT Reponse
 *
 * @param data Pointer to data*, Get buffer Pointer of Core Buffer
 * @param data_size Get Size
 * @return true if OK
 * @return false if failed
 */
bool netif_core_atcmd_get_data_after(uint8_t *data);

/**
 * @brief Reset AT Command Buffer and Response Result
 * 
 * @return true if Ok
 * @return false if false
 */
bool netif_core_atcmd_reset(bool reset_buffer);


// Specific Function

/**
 * @brief Input data from 4G Module to Netif Core
 * 
 * @param data Pointer of data
 * @param data_size Data size
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_core_4g_input(uint8_t* data, size_t data_size);

/**
 * @brief Output data from Netif Core, write to 4G Module
 * 
 * @param data Pointer to data
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed
 */
bool netif_core_4g_output(uint8_t* data, size_t data_size);

/**
 * @brief Input data from Wifi-Ethernet Device to Netif Core Stack
 * 
 * @param data Pointer to data
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed or Timeout
 */
bool netif_core_wifi_ethernet_input(uint8_t* data, size_t data_size);

/**
 * @brief Output data from Netif Core Stack, write to Wifi Ethernet Module
 * 
 * @param data Pointer to Data
 * @param data_size Data Size
 * @return true If OK
 * @return false If Failed or Timeout
 */
bool netif_core_wifi_ethernet_output(uint8_t * data, size_t data_size);

#endif
