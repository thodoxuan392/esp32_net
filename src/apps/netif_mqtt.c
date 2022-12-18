#include "apps/netif_mqtt.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_mqtt.h"
#include "utils/netif_buffer.h"
#include "utils/netif_logger.h"
#include "netif_opts.h"

// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Mqtt Client
static netif_mqtt_client_t* mqtt_client;

/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init(){
    return NETIF_OK;
}

/**
 * @brief Run MQTT Stack in supper loop, handle even, ...
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_run(){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    // Wait Connect AP Response
    if(netif_core_atcmd_is_responded(&at_response)){
        switch (at_response)
        {
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED:
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_connect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED:
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_disconnect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_SUB_OK:
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_subcribe(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK:
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_publish(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL:
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_publish(NETIF_FAIL);
            break;
        default:
            break;
        }
        
    }
    return NETIF_OK;
}

/**
 * @brief Deinitialize MQTT Stack
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_deinit(){
    return NETIF_OK;
}

// Specific Function
/**
 * @brief Config MQTT Client
 * 
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_config(netif_mqtt_client_t * client){
    // Set client to singleton mqtt_client
    mqtt_client = client;
}

/**
 * @brief Client connect to Broker
 * 
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_connect(netif_mqtt_client_t * client){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_MQTT_CONNECT,
                                        client->host,
                                        client->port,
                                        client->reconnect);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Reset Step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
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
 * @brief Disconnect from MQTT Broker
 * 
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_disconnect(netif_mqtt_client_t * client){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_MQTT_DISCONNECT);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Reset Step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
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
 * @brief Subcribe topic
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param qos Qos of Subcribe
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_MQTT_SUBCRIBE,
                                        topic,
                                        qos);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Reset Step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
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
 * @brief Unsubcribe topic
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_MQTT_UNSUBCRIBE,
                                        topic);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Reset Step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
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
 * @brief Publish Message to Topic 
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param payload Payload
 * @param qos Qos of Message
 * @param retain Retained Message
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain){
    static uint8_t step = 0;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_MQTT_PUBLISH,
                                        topic,
                                        payload,
                                        qos,
                                        retain);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Reset Step
            step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
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
 * @brief Check MQTT Client Connection
 * 
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_is_connected(netif_mqtt_client_t * client, bool *connected){
    *connected = client->connected;
    return NETIF_OK;
}
