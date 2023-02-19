#include "apps/netif_mqtt.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_mqtt.h"
#include "manager/netif_manager.h"
#include "netif_opts.h"
#include "utils/utils_buffer.h"
#include "utils/utils_logger.h"

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
static netif_status_t netif_wifi_ethernet_mqtt_parse_on_message();
static netif_status_t netif_4g_mqtt_parse_on_message();

/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init(){
	utils_log_debug("Netif Mqtt Init");
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
		case NETIF_4G_REPORT_MQTT_CONNECTED:
        	utils_log_info("Connected Callback");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_connect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED:
		case NETIF_4G_REPORT_MQTT_DISCONNECTED:
        	utils_log_info("Disconnected Callback");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_disconnect(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK:
			// Handle data when have on message for Wifi Ethernet
			netif_wifi_ethernet_mqtt_parse_on_message();
        	if(!on_message_process_flag){
				// Callback
				mqtt_client->on_message(topic,payload);
				// Reset Parameters
        		on_message_process_flag = true;
            	topic_len = 0;
            	payload_len = 0;
            	memset(topic,0,sizeof(topic));
            	memset(topic,0,sizeof(payload));
        	}
			break;
		case NETIF_4G_REPORT_MQTT_MESSAGE_OK:
			// Handle data when have on message for 4G
			netif_4g_mqtt_parse_on_message();
        	if(!on_message_process_flag){
				// Callback
				mqtt_client->on_message(topic,payload);
				// Reset Parameters
        		on_message_process_flag = true;
            	topic_len = 0;
            	payload_len = 0;
            	memset(topic,0,sizeof(topic));
            	memset(topic,0,sizeof(payload));
        	}
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK:
		case NETIF_4G_REPORT_MQTT_PUB_OK:
        	utils_log_info("Publish Callback OK");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            mqtt_client->on_publish(NETIF_OK);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL:
		case NETIF_4G_REPORT_MQTT_PUB_FAIL:
        	utils_log_info("Publish Callback Failed");
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
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
	utils_log_info("Netif Mqtt Deinit");
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
    static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	netif_core_response_t response;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send Client Config to AP to Wifi Module
					size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_CLIENT_CONFIG,
													1,
													client->client_id,
													client->username,
													client->password);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_ACCQ,
													client->client_id);
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
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
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	netif_core_response_t response;
	uint8_t connection_state;
	uint8_t* temp_ptr;
	uint8_t *data;
	size_t data_size;
    int size;
    switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send Connect to AP to Wifi Module
			    	size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_CONNECT,
													client->host,
													client->port,
													client->reconnect);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_CONNECT,
													client->host,
													client->port,
													client->keep_alive,
													client->clean_session,
													client->username,
													client->password);
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK){
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
 * * Step Description:
 * For Wifi-Ethernet interface:
 * 0: Disconnect
 * 1: Wait for get OK or Error response
 * For 4G interface:
 * 0: Disconnect
 * 1: Wait Get Ok or Error response.
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_disconnect(netif_mqtt_client_t * client){
	static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send Connect to AP to Wifi Module
			    	size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_DISCONNECT);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_DISCONNECT);
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
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
}

/**
 * @brief Subcribe topic
 *  * Step Description:
 * For Wifi-Ethernet interface:
 * 0: Unsubcribe Topic (Topic)
 * 1: Wait for get OK or Error response
 * For 4G interface:
 * 0: Indicate the topic len to subcribe
 * 1: Wait Input Event and Transmit Topic
 * 2: Wait Get Ok or Error response.
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
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send Subcribe Topic
			    	size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_SUBCRIBE,
															topic,
															qos);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_SUBSCRIBE_TOPIC,
															strlen(topic),
															qos);
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}else if(response == NETIF_RESPONSE_INPUT){
					netif_core_wifi_ethernet_output(topic, strlen(topic));
					step = 2;
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
			if(netif_core_atcmd_is_responded(&response)){
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
 * Step Description:
 * For Wifi-Ethernet interface:
 * 0: Unsubcribe Topic (Topic)
 * 1: Wait for get OK or Error response
 * For 4G interface:
 * 0: Indicate the topic len to unsubcribe
 * 1: Wait Input Event and Transmit Topic
 * 2: Wait Get Ok or Error response.
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
	static uint8_t step = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send unsubtopic 
					size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_UNSUBCRIBE,
				                                            	topic);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_UNSUBSCRIBE_TOPIC,
															strlen(topic));
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
				netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				netif_core_atcmd_get_data_before(&data, &data_size);
				// In the Wifi Ethernet Mode
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				// In the 4G Mode
				else if(response == NETIF_RESPONSE_INPUT){
					netif_core_wifi_ethernet_output(topic, strlen(topic));
					step = 2;
				}
				// Fail Case
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
			if(netif_core_atcmd_is_responded(&response)){
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
 * Step Description:
 * For Wifi-Ethernet interface:
 * 0: Publish Complete Message (Topic, Payload, qos, retain)
 * 1: Wait for get OK or Error response
 * For 4G interface:
 * 0: Indicate the topic len
 * 1: Wait Input Event and Transmit Topic
 * 2: Wait Get Ok or Error event. And Transmit Payload Len
 * 3: Wait Input Event and Transmit Payload
 * 4: Wait Get Ok or Error event.
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
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case 0:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				if(netmanager_mode == NETIF_MANAGER_WIFI_MODE || netmanager_mode == NETIF_MANAGER_ETHERNET_MODE){
					// Send Connect to AP to Wifi Module
					size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_PUBLISH,
													topic,
													payload,
													qos,
													retain);
				}else if(netmanager_mode == NETIF_MANAGER_4G_MODE){
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_PUBLISH_TOPIC,
															strlen(topic));
				}else{
					// Is Disconnect Mode -> Return FAIL code
					return NETIF_FAIL;
				}
			    netif_core_wifi_ethernet_output(at_message, size);
				step = 1;
			}
			break;
		case 1:
			if(netif_core_atcmd_is_responded(&response)){
				// For Wifi-Ethernet case
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = 0;
					return NETIF_OK;
				}
				// For 4G case -> Transmit Topic
				else if(response == NETIF_RESPONSE_INPUT){
					netif_core_wifi_ethernet_output(topic, strlen(topic));
					step = 2;
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
		case 2:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(false);
					// Transmit Payload Len
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_PUBLISH_PAYLOAD,
															strlen(payload));
					netif_core_wifi_ethernet_output(at_message, size);
					retry = 0;
					// Switch to wait for transmiting payload
					step = 3;
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
		case 3:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_INPUT){
					// Transmit Payload
					netif_core_wifi_ethernet_output(payload, strlen(payload));
					retry = 0;
					// Switch to Publish Message
					step = 4;
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
		case 4:
			if(netif_core_atcmd_is_responded(&response)){
				if(response == NETIF_RESPONSE_OK){
					// Reset Buffer
					netif_core_atcmd_reset(false);
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
static netif_status_t netif_wifi_ethernet_mqtt_parse_on_message(){
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


static netif_status_t netif_4g_mqtt_parse_on_message(){

}