#include "core/netif_core.h"
#include "utils/netif_buffer.h"
#include "utils/netif_string.h"

static const char * at_response_table[] = {
    [NETIF_RESPONSE_OK] = "OK",
    [NETIF_RESPONSE_ERROR] = "ERROR",
    [NETIF_RESPONSE_INPUT] = ">",
    [NETIF_WIFI_ETHERNET_RESPONSE_SEND_OK] = "SEND OK",
    [NETIF_WIFI_ETHERNET_RESPONSE_SEND_FAIL] = "SEND FAIL",
    [NETIF_WIFI_ETHERNET_RESPONSE_SET_OK] = "SET OK",
    [NETIF_WIFI_ETHERNET_REPORT_READY] = "ready",
    [NETIF_WIFI_ETHERNET_REPORT_BUSY] = "busy p...",
    [NETIF_WIFI_ETHERNET_REPORT_FORCE_RESTART] = "Will force to restart!!!",
    [NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_TYPE] = "smartconfig type",
    [NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_INFO] = "Smart get wifi info",
    [NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_CONNECTED_AP] = "smartconfig connected wifi",
    [NETIF_WIFI_ETHERNET_REPORT_WIFI_CONNECTED] = "WIFI CONNECTED",
    [NETIF_WIFI_ETHERNET_REPORT_WIFI_GOT_IP] = "WIFI GOT IP",
    [NETIF_WIFI_ETHERNET_REPORT_WIFI_DISCONNECTED] = "WIFI DISCONNECT",
    [NETIF_WIFI_ETHERNET_REPORT_ETH_CONNECTED] = "+ETH_CONNECTED",
    [NETIF_WIFI_ETHERNET_REPORT_ETH_GOT_IP] = "+ETH_GOT_IP",
    [NETIF_WIFI_ETHERNET_REPORT_ETH_DISCONNECTED] = "+ETH_DISCONNECTED",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED] = "+MQTTCONNECTED",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED] = "+MQTTDISCONNECTED",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_SUB_OK] = "+MQTTSUBRECV",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK] = "+MQTTPUB:OK",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL] = "+MQTTPUB:FAIL",
};
static uint16_t at_response_table_size = sizeof(at_response_table)/sizeof(at_response_table[0]);

// Netif Buffer to receive response from Module
static netif_buffer_t buffer_4g;
static netif_buffer_t buffer_wifi_ethernet;

// Core buffer for handling response
static uint8_t core_wifi_ethernet_buffer[BUFFER_MAX_SIZE];
static uint8_t core_wifi_ethernet_buffer_index = 0;
static bool at_response_indication= false;
static netif_core_response_t at_response;

// Internal Function
static void netif_core_process_response(){
    if(netif_buffer_is_available(&buffer_wifi_ethernet)){
        volatile uint8_t * data_p = &core_wifi_ethernet_buffer[core_wifi_ethernet_buffer_index++];
        netif_buffer_pop(&buffer_wifi_ethernet,data_p,1);
        // Check Response buffer match with any in at reponse table
        for (size_t i = 0; i < at_response_table_size; i++)
        {
            if (netif_string_is_receive_data(core_wifi_ethernet_buffer,
                                                core_wifi_ethernet_buffer_index,
                                                at_response_table[i])){
                // Match with index i 
                at_response = (netif_core_response_t)i;
                at_response_indication = true;
            }
        }
    }
}

// Generic Function
/**
 * @brief Initialize Network Interface Core Stack
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_core_init(){
    // Cleanup Buffer
    netif_buffer_init(&buffer_4g);
    netif_buffer_init(&buffer_wifi_ethernet);
}

/**
 * @brief Run Network Interface Core Stack in Supper Loop
 * 
 * @return true If OK
 * @return false If failed
 */
bool netif_core_run(){
    // Process Reponse from Module (4G or Wifi_Ethernet)
    netif_core_process_response();
}

/**
 * @brief Deinitialize Network Interface Core Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_core_deinit(){
    // Do something
}

/**
 * @brief Check ATCMD is have response match with atcommand table
 * 
 * @param response Pointer to response
 * @return true if have response
 * @return false if not
 */
bool netif_core_atcmd_is_responded(netif_core_response_t* response){
    if(at_response_indication){
        *response = at_response;
        return true;
    }
    return false;
}

/**
 * @brief Get Data if have at response
 * 
 * @param data Pointer to data*, Get buffer Pointer of Core Buffer
 * @param data_size Get Size
 * @return true if OK
 * @return false if failed
 */
bool netif_core_atcmd_get_data(uint8_t **data, size_t * data_size){
    if(at_response_indication){
        *data = core_wifi_ethernet_buffer;
        *data_size = core_wifi_ethernet_buffer_index;
        return true;
    }
    return false;
}

/**
 * @brief Reset AT Command Buffer and Response Result
 * 
 * @return true if Ok
 * @return false if false
 */
bool netif_core_atcmd_reset(){
    at_response_indication = false;
    core_wifi_ethernet_buffer_index = 0;
    return true;
}

/**
 * @brief Input data from 4G Module to Netif Core
 * 
 * @param data Pointer of data
 * @param data_size Data size
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_core_4g_input(uint8_t* data, size_t data_size){
    netif_buffer_push(&buffer_4g, data, data_size);
}

/**
 * @brief Output data from Netif Core, write to 4G Module
 * 
 * @param data Pointer to data
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed
 */
bool netif_core_4g_output(uint8_t* data, size_t data_size){
    NETIF_4G_OUTPUT(data, data_size);
}

/**
 * @brief Input data from Wifi-Ethernet Device to Netif Core Stack
 * 
 * @param data Pointer to data
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed or Timeout
 */
bool netif_core_wifi_ethernet_input(uint8_t* data, size_t data_size){
    netif_buffer_push(&buffer_wifi_ethernet, data, data_size);
}

/**
 * @brief Output data from Netif Core Stack, write to Wifi Ethernet Module
 * 
 * @param data Pointer to Data
 * @param data_size Data Size
 * @return true If OK
 * @return false If Failed or Timeout
 */
bool netif_core_wifi_ethernet_output(uint8_t * data, size_t data_size){
    NETIF_WIFI_ETHERNET_OUTPUT(data, data_size);
}





