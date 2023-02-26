#include "manager/netif_ethernet.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_ethernet.h"

#include "utils/utils_logger.h"


// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Ethernet Status
static bool ethernet_connected = false;
/**
 * @brief Initialize Ethernet Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_ethernet_init();

/**
 * @brief Run Ethernet Stack in Supper Loop, handle event,...
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_ethernet_run(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    // Check Wifi Event
    if(netif_core_atcmd_is_responded(&at_response)){
        switch (at_response)
        {
        case NETIF_WIFI_ETHERNET_REPORT_ETH_CONNECTED:
        	utils_log_info("Ethernet connected\r\n");
            /* code */
            ethernet_connected = true;
            netif_core_atcmd_reset(false);
            return NETIF_OK;
            break;
        case NETIF_WIFI_ETHERNET_REPORT_ETH_DISCONNECTED:
        	utils_log_info("Ethernet disconnected\r\n");
            /* code */
            ethernet_connected = false;
            netif_core_atcmd_reset(false);
            return NETIF_OK;
            break;
        default:
            break;
        }
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Deinitialize Ethernet Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_ethernet_deinit();


/**
 * @brief Check Ethernet Connected
 * 
 * @param connected Pointer to connected, if connected -> connected = true else false
 * @return netif_status_t NETIF_OK if OK, ...
 */
netif_status_t netif_ethernet_is_connected(bool *connected){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    int size;
    if(ethernet_connected){
        *connected = ethernet_connected;
        return NETIF_OK;
    }else{
    switch (step)
        {
        case 0:
        	// Clear All AT Response Before
        	netif_core_atcmd_reset(true);
            // Send Request Ethernet State to Module
            size = sprintf(at_message, NETIF_ATCMD_ETHERNET_GET_IP);
            netif_core_wifi_ethernet_output(at_message, size);
            // Switch wait to Wait Disconnect AP Response
            step = 1;
            break;
        case 1:
            // Wait Disconnect AP Response
            if(netif_core_atcmd_is_responded(&at_response)){
                // Reset State
                step = 0;
                // Check AT Response
                if(at_response == NETIF_RESPONSE_OK){
                    // Get Connection Status
                    netif_core_atcmd_get_data_before(&data , &data_size);
                    // Handling to get status from data
                    // TODO:
                    //*connected = true;
                    netif_core_atcmd_reset(false);
                    return NETIF_OK;
                }else{
                    // Donot use data from response -> Clean Core Buffer
                    netif_core_atcmd_reset(false);
                    return NETIF_FAIL;
                }
            }
            break;
        default:
            break;
        }
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Get Ethernet IP Address 
 * 
 * @param ip Pointer to IP String Buffer 
 * @param ip_max_size Max Size of IP String Buffer
 * @return true If Get IP OKe
 * @return false If Get Ip failed or Timeout
 */
netif_status_t netif_ethernet_get_ip(char * ip, size_t ip_max_size){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Ethernet Module
        size = sprintf(at_message, NETIF_ATCMD_ETHERNET_GET_IP);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Get Data from Core Buffer
            netif_core_atcmd_get_data_before(&data, &data_size);
            // Handle data to get IP
            // TODO: ...
            // Reset Core ATCMD buffer
            netif_core_atcmd_reset(false);
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
 * @brief Get Ethernet Mac Address
 * 
 * @param mac Pointer to MAC Address
 * @param mac_max_size Mac Size of Mac String Buffer
 * @return true 
 * @return false 
 */
netif_status_t netif_ethernet_get_mac(char * mac, size_t mac_max_size){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Ethernet Module
        size = sprintf(at_message, NETIF_ATCMD_ETHERNET_GET_MAC);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Get Data from Core Buffer
            netif_core_atcmd_get_data_before(&data, &data_size);
            // Handle data to get IP
            // TODO: ...
            // Reset Core ATCMD buffer
            netif_core_atcmd_reset(false);
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
