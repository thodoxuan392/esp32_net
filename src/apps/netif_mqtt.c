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
// Topic and Payload for On Message
static uint8_t topic[NETIF_MQTT_TOPIC_LEN];
static uint8_t payload[NETIF_MQTT_PAYLOAD_LEN];

/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init(){
	netif_log_info("Netif Mqtt Init");
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
    uint8_t *data;
    size_t data_len;
    // Wait Connect AP Response
    if(netif_core_atcmd_is_responded(&at_response)){
        switch (at_response)
        {
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED:
        	netif_log_info("Connected Callback");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_connect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED:
        	netif_log_info("Disconnected Callback");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_disconnect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK:
        	netif_log_info("Message Callback");
        	// Get data from atcommand
        	netif_core_atcmd_get_data(&data, &data_len);
        	// Handling to get topic and payload
        	//topic = NULL;
        	//payload = NULL;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_message(topic,payload);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK:
        	netif_log_info("Publish Callback OK");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset();
            mqtt_client->on_publish(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL:
        	netif_log_info("Publish Callback Failed");
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
	netif_log_info("Netif Mqtt Deinit");
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
    int size;
    // Send Connect to AP to Wifi Module
    size = sprintf(at_message, NETIF_ATCMD_MQTT_CONNECT,
                                    client->host,
                                    client->port,
                                    client->reconnect);
    netif_core_wifi_ethernet_output(at_message, size);
    return NETIF_OK;
}

/**
 * @brief Disconnect from MQTT Broker
 * 
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_disconnect(netif_mqtt_client_t * client){
    int size;
    // Send Disconnect to AP to Wifi Module
    size = sprintf(at_message, NETIF_ATCMD_MQTT_DISCONNECT);
	netif_core_wifi_ethernet_output(at_message, size);
    return NETIF_OK;
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
    int size;
    size = sprintf(at_message, NETIF_ATCMD_MQTT_SUBCRIBE,
                                            topic,
                                            qos);
	netif_core_wifi_ethernet_output(at_message, size);
    return NETIF_OK;
}

/**
 * @brief Unsubcribe topic
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
    int size;
    size = sprintf(at_message, NETIF_ATCMD_MQTT_UNSUBCRIBE,
                                            topic);
	netif_core_wifi_ethernet_output(at_message, size);
    return NETIF_OK;
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
    int size;
    size = sprintf(at_message, NETIF_ATCMD_MQTT_PUBLISH,
                                    topic,
                                    payload,
                                    qos,
                                    retain);
    netif_core_wifi_ethernet_output(at_message, size);
    return NETIF_OK;
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
