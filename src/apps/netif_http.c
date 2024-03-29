#include "apps/netif_http.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_http.h"
#include "netif_opts.h"
#include "utils/utils_buffer.h"
#include "utils/utils_logger.h"


// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];

/**
 * @brief Initialize HTTP Apps 
 * 
 * @return true if OK
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_init(){
	utils_log_debug("Netif Http Init\r\n");
     // Do nothing
     return NETIF_OK;
}

/**
 * @brief Run HTTP Stack in Super Loop, handle event, ...
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_run(){
    // Do nothing
}


/**
 * @brief Deinitialize HTTP Apps 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_deinit(){
	utils_log_info("Netif Http Deinit\r\n");
    // Do nothing
    return NETIF_OK;
}

// Specific Function
/**
 * @brief Send HTTP GET request
 * 
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_get_request(netif_http_request_t * request){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_HTTP_GET,
                                        request->url);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response)){
            // Reset step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                // Switch to Send Post data
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}




/**
 * @brief Send HTTP POST request
 * 
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_post_request(netif_http_request_t * request){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_HTTP_POST,
                                        request->url,
                                        strlen(request->data));
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to response
        step = 1;
        break;
    case 1:
        // Wait HTTP Post Response
        if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                // Wait for receive input indicator
                step = 2;
            }else if (at_response == NETIF_RESPONSE_OK){
                // Reset step if fail
                step = 0;
                return NETIF_FAIL;
            }
        }
        break;
    case 2:
        if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_INPUT){
                // Send Post data
                step = 3;
            }else{
                // Reset step if fail
                step = 0;
                return NETIF_FAIL;
            }
        }
        break;
    case 3:
        // Send Post data to Server
        netif_core_wifi_ethernet_output(request->data, strlen(request->data));
        // Switch wait to response
        step = 4;
        break;
    case 4:
        // Wait HTTP Post Response
        if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
            // Check AT Response
            if(at_response == NETIF_WIFI_ETHERNET_RESPONSE_SEND_OK){
                // Reset step 
                step = 0;
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}


/**
 * @brief Send HTTP PUT request
 * 
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_put_request(netif_http_request_t * request);
