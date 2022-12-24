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
static uint16_t topic_len;
static uint8_t payload[NETIF_MQTT_PAYLOAD_LEN];
static uint16_t payload_len;
static bool on_message_process_flag = false;

// Internal Function
static netif_status_t netif_mqtt_parse_on_message();

/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init(){
	netif_log_debug("Netif Mqtt Init");
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
            netif_core_atcmd_reset(false);
            mqtt_client->on_connect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED:
        	netif_log_info("Disconnected Callback");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_disconnect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK:
        	// Enable for prcocess on message
        	if(!on_message_process_flag){
        		on_message_process_flag = true;
            	topic_len = 0;
            	payload_len = 0;
            	memset(topic,0,sizeof(topic));
            	memset(topic,0,sizeof(payload));
        	}

            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK:
        	netif_log_info("Publish Callback OK");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_publish(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL:
        	netif_log_info("Publish Callback Failed");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_publish(NETIF_FAIL);
            break;
        default:
            break;
        }
        
    }
    // Handle data when have on message
    netif_mqtt_parse_on_message();
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
 * 0: Deinit Previous CLient
 * 1: Waiting Response
 * 2: Init new client
 * 3: Waiting Response
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_config(netif_mqtt_client_t * client){
    // Set client to singleton mqtt_client
    mqtt_client = client;
    static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				// Send Connect to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_MQTT_CLIENT_CONFIG,
												1,
												client->client_id,
												client->username,
												client->password);
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK ||
						response == NETIF_RESPONSE_ERROR){	// Force Error is Sucess because don't have read command to get client status
					netif_core_atcmd_reset(true);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

/**
 * @brief Client connect to Broker
 * 
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_connect(netif_mqtt_client_t * client){
	static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
    int size;
    switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				// Send Connect to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_MQTT_CONNECT_QUERY);
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK){
					//Get AT Response Buffer
					netif_core_atcmd_get_data_before(&data, &data_size);
					// Check whether MQTT Client connect before or not: pattern +MQTTCONN:
					if(strnstr(data,"+MQTTCONN:",data_size)){
						// Connected before -> Return OK
						netif_core_atcmd_reset(true);
						step = 0;
						retry = 0;
						return NETIF_OK;
					}else{
						// Not connected before -> Step 2 to connect
						retry = 0;
						step = 2;
					}
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
				}
			}
			break;
		case 2:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
			    // Send Connect to AP to Wifi Module
			    size = sprintf(at_message, NETIF_ATCMD_MQTT_CONNECT,
			                                    client->host,
			                                    client->port,
			                                    client->reconnect);
			    netif_core_wifi_ethernet_output(at_message, size);
			    step = 1;
			}
			break;
		case 3:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK ||
						response == NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED){
					netif_core_atcmd_reset(true);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
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
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Send Disconnect to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_MQTT_DISCONNECT);
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK ||
						response == NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED){
					netif_core_atcmd_reset(true);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
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
	static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				// Send Connect to AP to Wifi Module
			    size = sprintf(at_message, NETIF_ATCMD_MQTT_SUBCRIBE,
			                                            topic,
			                                            qos);
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
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
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				// Send Connect to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_MQTT_UNSUBCRIBE,
				                                            topic);
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
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
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Send Connect to AP to Wifi Module
			    size = sprintf(at_message, NETIF_ATCMD_MQTT_PUBLISH,
			                                    topic,
			                                    payload,
			                                    qos,
			                                    retain);
			    netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						// Reset Buffer and Indication if Try number over
						netif_core_atcmd_reset(true);
						retry = 0;
						return NETIF_FAIL;
					}
					retry ++;
					step = 0;
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

// Internal Function

/**
 * @brief Parse On Message
 * Step
 * 0: Get LinkID	// Ignore
 * 1: Get Topic
 * 2: Get Payload len
 * 3: Get Payload
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_mqtt_parse_on_message(){
	static char payload_length_buffer[16];
	static uint8_t payload_length_index = 0;
	static uint8_t step = 0;
	static uint8_t start_get_topic = false;
	static uint8_t start_get_payload = false;
	uint8_t data;
	if(on_message_process_flag){
		switch (step) {
			case 0:
				if(netif_core_atcmd_get_data_after(&data)){
					if(data == ','){
						step = 1;
					}
				}
				break;
			case 1:
				if(netif_core_atcmd_get_data_after(&data)){
					if(data == ','){
						step = 2;
					}else if(data != '"' ){
						topic[topic_len++] = data;
					}
				}
				break;
			case 2:
				if(netif_core_atcmd_get_data_after(&data)){
					if(data == ','){
						step = 3;
					}
				}
				break;
			case 3:
				if(netif_core_atcmd_get_data_after(&data)){
					if(data == '\n'){
						mqtt_client->on_message(topic,payload);
						on_message_process_flag = false;
						netif_core_atcmd_reset(false);
						step = 0;
					}else{
						payload[payload_len++] = data;
					}
				}
				break;
			default:
				break;
		}
	}
}
