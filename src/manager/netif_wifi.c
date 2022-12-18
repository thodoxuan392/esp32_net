#include "manager/netif_wifi.h"
#include "core/netif_core.h"
#include "atcmd/netif_atcmd_wifi.h"
#include "utils/netif_buffer.h"
#include "utils/netif_logger.h"
#include "netif_opts.h"

// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Wifi Status
static bool wifi_connected = false;
static char smartconfig_ssid[SSID_LEN];
static char smartconfig_password[PASSWORD_LEN];
static char wifi_ip[IP_ADDR_LEN];
static char wifi_mac[MAC_ADDR_LEN];

// Generic Function
/**
 * @brief Initialize Wifi Stack
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_init(){
    return NETIF_OK;
}

/**
 * @brief Run Wifi Stack in Supper loop, handle event, ...
 * * Step:
 * 0: Enable Station Mode
 * 1: Wait Station Mode response
 * 2: Check Wifi Event
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_run(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    switch (step)
    {
    case 0:
        // Enable Station Mode
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_STATION_MODE);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to 
        step = 1;
        break;
    case 1:
        // Wait Station Mode Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
            step = 2;
        }
        break;
    case 2:
        // Check Wifi Event
        if(netif_core_atcmd_is_responded(&at_response)){
            switch (at_response)
            {
            case NETIF_WIFI_ETHERNET_REPORT_WIFI_CONNECTED:
                /* code */
                wifi_connected = true;
                netif_core_atcmd_reset();
                break;
            case NETIF_WIFI_ETHERNET_REPORT_WIFI_DISCONNECTED:
                /* code */
                wifi_connected = false;
                netif_core_atcmd_reset();
                break;
            case NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_CONNECTED_AP:
                /* code */
                // Get data from Core Buffer
                netif_core_atcmd_get_data(&data,&data_size);
                // Get SSID and Pass
                // Reset ATCMD
                netif_core_atcmd_reset();
                break;
            default:
                break;
            }
        }
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Deinitialize Wifi Stack
 * @return true if OK
 * @return false if failed
 */
netif_status_t netif_wifi_deinit();

// Specific Function
/**
 * @brief Connect to SoftAP using SSID and Password
 * Step:
 * 0: Send request connect to Wifi Module
 * 1: Wait for response
 * @param ssid SSID of SoftAP
 * @param password Password of SoftAP
 * @param auth_mode Authentication Mode of SoftAP
 * @return true if connected
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_connect_ap(char *ssid, char * password){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_CONNECT_AP, ssid, password);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Disconnect from AP
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_disconnect_ap(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_DISCONNECT_AP);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_is_connected(bool * connected){
    * connected = wifi_connected;
    return NETIF_OK; 
}

/**
 * @brief Start SmartConfig 
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_start_smartconfig(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_START_SMARTCONFIG);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Stop Smartconfig
 * 
 * @return true  If OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_stop_smartconfig(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_STOP_SMARTCONFIG);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait ATCMD Reponse
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Get Ip Address of Wifi Station
 * 
 * @param ip Pointer to Ip String Buffer
 * @param ip_max_size Ip String Buffer Max Size
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_get_ip(char *ip , size_t ip_max_size){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_GET_IP);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Get Data from Core Buffer
            netif_core_atcmd_get_data(&data, &data_size);
            // Handle data to get IP
            // TODO: ...
            // Reset Core ATCMD buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Get Mac Address of Wifi Station
 * 
 * @param mac Pointer to Mac String Buffer
 * @param mac_max_size Mac String Buffer Max Size
 * @return true 
 * @return false 
 */
netif_status_t netif_wifi_get_mac(char *mac , size_t mac_max_size){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        int size = sprintf(at_message, NETIF_ATCMD_WIFI_GET_MAC);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait ATCMD Reponse
        step = 1;
        break;
    case 1:
        // Wait ATCMD Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Get Data from Core Buffer
            netif_core_atcmd_get_data(&data, &data_size);
            // Handle data to get MAC
            // TODO: ...
            // Reset Core ATCMD buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}