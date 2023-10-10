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
// On message buffer process
static uint8_t on_message_buffer[NETIF_MQTT_ON_MESSAGE_LEN];

/******************************** Internal Function***********************************/
// Request State
enum {
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
	// Wifi Ethernet Request State
	STATE_WIFI_ETHERNET_MQTT_CONFIG,
	STATE_WIFI_ETHERNET_MQTT_CONNECT,
	STATE_WIFI_ETHERNET_MQTT_DISCONNECT,
	STATE_WIFI_ETHERNET_MQTT_SUBCRIBE,
	STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE,
	STATE_WIFI_ETHERNET_MQTT_PUBLISH,
	STATE_WIFI_ETHERNET_MQTT_PUBLISH_INPUT,
	STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE,
#endif

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
	// 4G Request State
	STATE_4G_MQTT_START,
	STATE_4G_MQTT_CONFIG,
	STATE_4G_MQTT_CONNECT,
	STATE_4G_MQTT_DISCONNECT,
	STATE_4G_MQTT_SUBCRIBE_TOPIC,
	STATE_4G_MQTT_SUBCRIBE_TOPIC_INPUT,
	STATE_4G_MQTT_UNSUBCRIBE_TOPIC,
	STATE_4G_MQTT_UNSUBCRIBE_TOPIC_INPUT,
	STATE_4G_MQTT_PUBLISH_TOPIC,
	STATE_4G_MQTT_PUBLISH_TOPIC_INPUT,
	STATE_4G_MQTT_PUBLISH_PAYLOAD,
	STATE_4G_MQTT_PUBLISH_PAYLOAD_INPUT,
	STATE_4G_MQTT_PUBLISH,
	STATE_4G_MQTT_WAIT_FOR_RESPONSE
#endif
};

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
static bool netif_wifi_ethernet_mqtt_parse_on_message();
// Client Config
static netif_status_t netif_wifi_ethernet_mqtt_config(netif_mqtt_client_t * client);
// Client Connect
static netif_status_t netif_wifi_ethernet_mqtt_connect(netif_mqtt_client_t * client);
// Client Disconnect
static netif_status_t netif_wifi_ethernet_mqtt_disconnect(netif_mqtt_client_t * client);
// Client Subcribe
static netif_status_t netif_wifi_ethernet_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos);
// Client Unsubcribe
static netif_status_t netif_wifi_ethernet_mqtt_unsubcribe(netif_mqtt_client_t * client, char * topic);
// Client Publish
static netif_status_t netif_wifi_ethernet_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain);
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
static bool netif_4g_mqtt_parse_on_message();
// Client Config
static netif_status_t netif_4g_mqtt_config(netif_mqtt_client_t * client);
// Client Connect
static netif_status_t netif_4g_mqtt_connect(netif_mqtt_client_t * client);
// Client Disconnect
static netif_status_t netif_4g_mqtt_disconnect(netif_mqtt_client_t * client);
// Client Subcribe
static netif_status_t netif_4g_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos);
// Client Unsubcribe
static netif_status_t netif_4g_mqtt_unsubcribe(netif_mqtt_client_t * client, char * topic);
// Client Publish
static netif_status_t netif_4g_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain);
#endif



/******************************** Public Function***********************************/
/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init(){
	utils_log_debug("Netif Mqtt Init\r\n");
    return NETIF_OK;
}

/**
 * @brief Run MQTT Stack in supper loop, handle even, ...
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_run(){
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_len;
    // Wait Connect AP Response
    if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response) || netif_core_atcmd_is_responded(NETIF_4G, &at_response)){
        switch (at_response)
        {
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_CONNECTED:
		case NETIF_4G_REPORT_MQTT_CONNECTED:
            // Donot use data from response -> Clean Core Buffer
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
			netif_core_atcmd_reset(NETIF_4G,false);
#endif
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
			netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
#endif
			if(mqtt_client && mqtt_client->on_connect){
            	mqtt_client->on_connect(NETIF_OK);
            }
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED:
		case NETIF_4G_REPORT_MQTT_DISCONNECTED:
            // Donot use data from response -> Clean Core Buffer
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
			netif_core_atcmd_reset(NETIF_4G,false);
#endif
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
			netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
#endif
			if(mqtt_client && mqtt_client->on_disconnect){
            	mqtt_client->on_disconnect(NETIF_OK);
            }
            break;
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK:
			// Handle data when have on message for Wifi Ethernet

        	if(netif_wifi_ethernet_mqtt_parse_on_message()){
        		// Clear AT Response
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
        		if(mqtt_client && mqtt_client->on_message){
				 mqtt_client->on_message(topic,payload);
				}
				// Reset Parameters
            	topic_len = 0;
            	payload_len = 0;
            	memset(topic,0,sizeof(topic));
            	memset(topic,0,sizeof(payload));
        	}
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G_REPORT_MQTT_MESSAGE_OK:
			// Handle data when have on message for 4G
			if(netif_4g_mqtt_parse_on_message()){
				// Clear AT response
				netif_core_atcmd_reset(NETIF_4G,false);
				// Callback
				if(mqtt_client && mqtt_client->on_message){
					mqtt_client->on_message(topic,payload);
				}
				// Reset Parameters
				topic_len = 0;
				payload_len = 0;
				memset(topic,0,sizeof(topic));
				memset(topic,0,sizeof(payload));
			}
            break;
#endif

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK:
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G_REPORT_MQTT_PUB_OK:
			netif_core_atcmd_reset(NETIF_4G,false);
#endif
            // Donot use data from response -> Clean Core Buffer

			netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
			if(mqtt_client && mqtt_client->on_publish){
            	mqtt_client->on_publish(NETIF_OK);
            }
            break;
        case NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL:
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G_REPORT_MQTT_PUB_FAIL:
			netif_core_atcmd_reset(NETIF_4G,false);
#endif
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
            if(mqtt_client && mqtt_client->on_publish){
            	mqtt_client->on_publish(NETIF_FAIL);
            }
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
	utils_log_info("Netif Mqtt Deinit\r\n");
    return NETIF_OK;
}

// Specific Function

/**
 * @brief Config MQTT Client
 * 0: Init new client
 * 1: Waiting Response
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_config(netif_mqtt_client_t * client){
	// Assign to System Mqtt Client
	mqtt_client = client;
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_config(client);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_config(client);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}

/**
 * @brief Client connect to Broker
 *
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_connect(netif_mqtt_client_t * client){
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_connect(client);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_connect(client);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}

/**
 * @brief Disconnect from MQTT Broker
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_disconnect(netif_mqtt_client_t * client){
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_disconnect(client);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_disconnect(client);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}


/**
 * @brief Subcribe topic
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param qos Qos of Subcribe
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos){
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_subcribe(client, topic, qos);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_subcribe(client, topic, qos);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}


/**
 * @brief Unsubcribe topic
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_unsubcribe(client, topic);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_unsubcribe(client, topic);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}



/**
 * @brief Publish Message to Topic
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param payload Payload
 * @param qos Qos of Message
 * @param retain Retained Message
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain){
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch (netmanager_mode) {
		case NETIF_MANAGER_WIFI_MODE:
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_mqtt_publish(client, topic, payload, qos, retain);
			break;
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1

		case NETIF_MANAGER_4G_MODE:
			return netif_4g_mqtt_publish(client, topic, payload, qos, retain);
			break;
#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
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


#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
/************************************* Wifi Ethernet Stack *************************************/

/**
 * @brief Config MQTT Client in Wifi Ethernet Stack
 * STATE_WIFI_ETHERNET_MQTT_CONFIG: Init new client
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE: Waiting Response
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_config(netif_mqtt_client_t * client){
    static uint8_t state = STATE_WIFI_ETHERNET_MQTT_CONFIG;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_CONFIG:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Client Config to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_CLIENT_CONFIG,
													1,
													client->client_id,
													client->username,
													client->password);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_CONFIG;
				utils_log_error("Mqtt config timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET,&response)){
				if(response == NETIF_RESPONSE_OK ||
						response == NETIF_RESPONSE_ERROR){	// Force Error is Sucess because don't have read command to get client status
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_CONFIG;
					return NETIF_OK;
				}
				else if(response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_CONFIG;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_CONFIG;
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
 * State Description:
 * STATE_WIFI_ETHERNET_MQTT_CONNECT: Connect to Mqtt Broker
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE: Waiting Response
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_connect(netif_mqtt_client_t * client){
	static uint8_t state = STATE_WIFI_ETHERNET_MQTT_CONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
    int size;
    switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_CONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Connect to AP to Wifi Ethernet Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_CONNECT,
												client->host,
												client->port,
												client->reconnect);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT * 4){
				state = STATE_WIFI_ETHERNET_MQTT_CONNECT;
				utils_log_error("Mqtt connect timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_CONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_CONNECT;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_CONNECT;
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
 * State Description:
 * STATE_WIFI_ETHERNET_MQTT_DISCONNECT : Disconnect from Mqtt Broker
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE : Waiting Response
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_disconnect(netif_mqtt_client_t * client){
	static uint8_t state = STATE_WIFI_ETHERNET_MQTT_DISCONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_DISCONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Connect to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_DISCONNECT);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_DISCONNECT;
				utils_log_error("Mqtt disconnect timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response)){
				if(response == NETIF_RESPONSE_OK ||
						response == NETIF_WIFI_ETHERNET_REPORT_MQTT_DISCONNECTED){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_DISCONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_DISCONNECT;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_DISCONNECT;
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
 * State Description:
 * STATE_WIFI_ETHERNET_MQTT_SUBCRIBE: Subcribe Topic (Topic)
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param qos Qos of Subcribe
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos){
	static uint8_t state = STATE_WIFI_ETHERNET_MQTT_SUBCRIBE;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_SUBCRIBE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Subcribe Topic
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_SUBCRIBE,
														topic,
														qos);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_SUBCRIBE;
				utils_log_error("Mqtt subscribe timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_SUBCRIBE;
					return NETIF_OK;
				}else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_SUBCRIBE;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_SUBCRIBE;
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
 * State Description:
 * STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE: Unsubcribe Topic (Topic)
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
	static uint8_t state = STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Unsubcribe Request to Wifi Ethernet Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_UNSUBCRIBE,
															topic);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE;
				utils_log_error("Mqtt unsubscribe timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET,&response)){
				// In the Wifi Ethernet Mode
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE;
					return NETIF_OK;
				}
				// Fail Case
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_UNSUBCRIBE;
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
 * State Description:
 * STATE_WIFI_ETHERNET_MQTT_PUBLISH: Publish Message
 * STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param payload Payload
 * @param qos Qos of Message
 * @param retain Retained Message
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_wifi_ethernet_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain){
    static uint8_t state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_WIFI_ETHERNET_MQTT_PUBLISH:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
				// Send Publish Request to Wifi Ethernet Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_MQTT_PUBLISH,
																	topic,
																	strlen(payload),
																	qos,
																	retain);
			    netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_MQTT_PUBLISH_INPUT;
			}
			break;
		case STATE_WIFI_ETHERNET_MQTT_PUBLISH_INPUT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
				utils_log_error("Mqtt publish timeout to receive input\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET,&response)){
				if(response == NETIF_RESPONSE_INPUT){
					last_time_sent = NETIF_GET_TIME_MS();
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
					netif_core_wifi_ethernet_output(payload, strlen(payload));
					retry = 0;
					state = STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						// Reset Buffer and Indication if Try number over
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
				}
				// Ignore other case
				else{
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
				}
			}
			break;
		case STATE_WIFI_ETHERNET_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
				utils_log_error("Mqtt publish timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response)){
				// For Wifi-Ethernet case
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY
						 || response == NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL){
					if(retry >= NETIF_MAX_RETRY){
						// Reset Buffer and Indication if Try number over
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,true);
						retry = 0;
						state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_WIFI_ETHERNET_MQTT_PUBLISH;
				}else{
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET,false);
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}


#endif

/************************************* 4G Stack *************************************/
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
/**
 * @brief Config MQTT Client in 4G Stack
 * State Description:
 * STATE_4G_MQTT_CONFIG: Init new client
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE: Waiting Response
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_config(netif_mqtt_client_t * client){
    static uint8_t state = STATE_4G_MQTT_START;
    static uint8_t started_flag = 0;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_4G_MQTT_START:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Client Config to AP to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_START);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_MQTT_CONFIG:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Client Config to AP to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_ACCQ,
														client->client_id);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				if(retry >= NETIF_MAX_RETRY){
					netif_core_atcmd_reset(NETIF_4G ,false);
					retry = 0;
					started_flag = 0;
					state = STATE_4G_MQTT_START;
					return NETIF_FAIL;
				}
				retry ++;
				started_flag = 0;
				state = STATE_4G_MQTT_START;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
				if(response == NETIF_RESPONSE_OK ){
					netif_core_atcmd_reset(NETIF_4G ,true);
					if(!started_flag){
						started_flag = 1;
						state = STATE_4G_MQTT_CONFIG;
					}else{
						started_flag = 0;
						retry = 0;
						state = STATE_4G_MQTT_START;
						return NETIF_OK;
					}
				}
				else if(response == NETIF_RESPONSE_ERROR){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_4G ,false);
						retry = 0;
						started_flag = 0;
						state = STATE_4G_MQTT_START;
						return NETIF_FAIL;
					}
					retry ++;
					started_flag = 0;
					state = STATE_4G_MQTT_START;
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
 * State Description:
 * STATE_4G_MQTT_CONNECT: Connect to Mqtt Broker
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE: Waiting Response
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_connect(netif_mqtt_client_t * client){
	static uint8_t state = STATE_4G_MQTT_CONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
    switch (state) {
		case STATE_4G_MQTT_CONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_CONNECT,
																	client->host,
																	client->port,
																	client->keep_alive,
																	client->clean_session,
																	client->username,
																	client->password);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G ,false);
				if(retry >= NETIF_MAX_RETRY){
					retry = 0;
					state = STATE_4G_MQTT_CONNECT;
					return NETIF_FAIL;
				}
				retry ++;
				state = STATE_4G_MQTT_CONNECT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_4G ,true);
					retry = 0;
					state = STATE_4G_MQTT_CONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G ,false);
					if(retry >= NETIF_MAX_RETRY){
						retry = 0;
						state = STATE_4G_MQTT_CONNECT;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_CONNECT;
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
 * State Description:
 * STATE_4G_MQTT_DISCONNECT : Disconnect from Mqtt Broker
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE : Waiting Response
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_disconnect(netif_mqtt_client_t * client){
	static uint8_t state = STATE_4G_MQTT_DISCONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_4G_MQTT_DISCONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Disconnect Request to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_DISCONNECT);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G ,false);
				if(retry >= NETIF_MAX_RETRY){
					retry = 0;
					state = STATE_4G_MQTT_DISCONNECT;
					return NETIF_FAIL;
				}
				retry ++;
				state = STATE_4G_MQTT_DISCONNECT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G ,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_4G ,true);
					retry = 0;
					state = STATE_4G_MQTT_DISCONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G ,false);
					if(retry >= NETIF_MAX_RETRY){
						retry = 0;
						state = STATE_4G_MQTT_DISCONNECT;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_DISCONNECT;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}




/**
 * @brief Subcribe topic Request in 4G Module
 * State Description:
 * STATE_4G_MQTT_SUBCRIBE_TOPIC: Send Topic Len
 * STATE_4G_MQTT_SUBCRIBE_TOPIC_INPUT: Send Topic Data
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param qos Qos of Subcribe
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos){
	static uint8_t state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_4G_MQTT_SUBCRIBE_TOPIC:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Subcribe Request to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_SUBSCRIBE,
																			strlen(topic),
																			qos);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_SUBCRIBE_TOPIC_INPUT;
			}
			break;
		case STATE_4G_MQTT_SUBCRIBE_TOPIC_INPUT:
			if(netif_core_atcmd_is_responded(NETIF_4G ,&response)){
				if(response == NETIF_RESPONSE_INPUT){
					// Clear Before Data
					netif_core_atcmd_reset(NETIF_4G ,true);
					netif_core_4g_output(topic, strlen(topic));
					retry = 0;
					state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(NETIF_4G ,false);
						retry = 0;
						state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G ,true);
				if(retry >= NETIF_MAX_RETRY){
					retry = 0;
					state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
					return NETIF_FAIL;
				}
				retry ++;
				state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G ,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_4G ,true);
					retry = 0;
					state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G ,true);
					if(retry >= NETIF_MAX_RETRY){
						retry = 0;
						state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_SUBCRIBE_TOPIC;
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
 * State Description:
 * STATE_4G_MQTT_UNSUBCRIBE_TOPIC: Send Topic Len
 * STATE_4G_MQTT_UNSUBCRIBE_TOPIC_INPUT: Send Topic Data
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic){
	static uint8_t state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_4G_MQTT_UNSUBCRIBE_TOPIC:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G ,true);
				// Send Unsubcribe Request to 4G module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_UNSUBSCRIBE_TOPIC,
																			strlen(topic));
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC_INPUT;
			}
			break;
		case STATE_4G_MQTT_UNSUBCRIBE_TOPIC_INPUT:
			if(netif_core_atcmd_is_responded(NETIF_4G ,&response)){
				if(response == NETIF_RESPONSE_INPUT){
					netif_core_4g_output(topic, strlen(topic));
					retry = 0;
					state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
				}
				// Fail Case
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G ,false);
					if(retry >= NETIF_MAX_RETRY){
						retry = 0;
						state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G,false);
				if(retry >= NETIF_MAX_RETRY){
					retry = 0;
					state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
					return NETIF_FAIL;
				}
				retry ++;
				state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_4G,false);
					retry = 0;
					state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G,false);
					if(retry >= NETIF_MAX_RETRY){
						retry = 0;
						state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
						return NETIF_FAIL;
					}
					retry ++;
					state = STATE_4G_MQTT_UNSUBCRIBE_TOPIC;
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
 * State Description:
 * STATE_4G_MQTT_PUBLISH_TOPIC: Send Topic Len
 * STATE_4G_MQTT_PUBLISH_TOPIC_INPUT: Wait Input Event and Transmit Topic
 * STATE_4G_MQTT_PUBLISH_PAYLOAD: Send Payload Len
 * STATE_4G_MQTT_PUBLISH_PAYLOAD_INPUT: Wait Input Event and Transmit Payload
 * STATE_4G_MQTT_WAIT_FOR_RESPONSE: Wait for response
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param payload Payload
 * @param qos Qos of Message
 * @param retain Retained Message
 * @return netif_status_t Status of Process
 */
static netif_status_t netif_4g_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain){
    static uint8_t state = STATE_4G_MQTT_PUBLISH_TOPIC;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
		case STATE_4G_MQTT_PUBLISH_TOPIC:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				// Clear rx buffer before publish
				netif_core_atcmd_reset(NETIF_4G ,true);
				last_time_sent = NETIF_GET_TIME_MS();
				// Send Publish Topic Len to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_PUBLISH_TOPIC,
																		strlen(topic));
				netif_core_4g_output(at_message, size);
				state = STATE_4G_MQTT_PUBLISH_TOPIC_INPUT;
			}
			break;
		case STATE_4G_MQTT_PUBLISH_TOPIC_INPUT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G ,true);
				if(retry >= NETIF_MAX_RETRY){
					utils_log_error("MQTT publish topic input response timeout, maximum retried time\r\n");
					// Reset Buffer and Indication if Try number over
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_FAIL;
				}
				utils_log_warn("MQTT publish topic input response failed, retrying ...\r\n");
				retry ++;
				state = STATE_4G_MQTT_PUBLISH_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G ,&response)){
				if(response == NETIF_RESPONSE_INPUT){
					netif_core_atcmd_reset(NETIF_4G ,true);
					last_time_sent = NETIF_GET_TIME_MS();
					netif_core_4g_output(topic, strlen(topic));
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_PAYLOAD;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					utils_log_error("MQTT publish topic input got error, maximum retried time\r\n");
					netif_core_atcmd_reset(NETIF_4G ,true);
					if(retry >= NETIF_MAX_RETRY){
						// Reset Buffer and Indication if Try number over
						retry = 0;
						state = STATE_4G_MQTT_PUBLISH_TOPIC;
						return NETIF_FAIL;
					}
					utils_log_error("MQTT publish topic input got error, retrying ...\r\n");
					retry ++;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_PUBLISH_PAYLOAD:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G ,true);
				if(retry >= NETIF_MAX_RETRY){
					utils_log_error("MQTT publish payload timeout, maximum retried time\r\n");
					// Reset Buffer and Indication if Try number over
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_FAIL;
				}
				utils_log_warn("MQTT publish payload timeout, retrying ...\r\n");
				retry ++;
				state = STATE_4G_MQTT_PUBLISH_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset( NETIF_4G , true);
					last_time_sent = NETIF_GET_TIME_MS();
					// Send Publish Payload Len to 4G Module
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_PUBLISH_PAYLOAD,
															strlen(payload));
					netif_core_4g_output(at_message, size);
					retry = 0;
					// Switch to wait for transmiting payload
					state = STATE_4G_MQTT_PUBLISH_PAYLOAD_INPUT;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G ,true);
					if(retry >= NETIF_MAX_RETRY){
						utils_log_error("MQTT publish payload got error, maximum retried time\r\n");
						retry = 0;
						state = STATE_4G_MQTT_PUBLISH_TOPIC;
						return NETIF_FAIL;
					}
					utils_log_error("MQTT publish payload got error, retrying ...\r\n");
					retry ++;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_PUBLISH_PAYLOAD_INPUT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G,true);
				if(retry >= NETIF_MAX_RETRY){
					utils_log_error("MQTT publish payload input timeout, maximum retried time\r\n");
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_FAIL;
				}
				utils_log_warn("MQTT publish payload input timeout, retrying ...\r\n");
				retry ++;
				state = STATE_4G_MQTT_PUBLISH_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_INPUT){
					// Transmit Payload
					netif_core_atcmd_reset(NETIF_4G ,true);
					last_time_sent = NETIF_GET_TIME_MS();
					netif_core_4g_output(payload, strlen(payload));
					retry = 0;
					// Switch to Publish Message
					state = STATE_4G_MQTT_PUBLISH;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G,true);
					if(retry >= NETIF_MAX_RETRY){
						utils_log_error("MQTT publish payload input got error, maximum retried time\r\n");
						retry = 0;
						state = STATE_4G_MQTT_PUBLISH_TOPIC;
						return NETIF_FAIL;
					}
					utils_log_warn("MQTT publish payload input got error, retrying ...\r\n");
					retry ++;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_PUBLISH:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G,true);
				if(retry >= NETIF_MAX_RETRY){
					utils_log_error("MQTT publish timeout, maximum retried time\r\n");
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_FAIL;
				}
				utils_log_warn("MQTT publish timeout, retrying ...\r\n");
				retry ++;
				state = STATE_4G_MQTT_PUBLISH_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_OK){
					netif_core_atcmd_reset(NETIF_4G,true);
					last_time_sent = NETIF_GET_TIME_MS();
					// Publish Message
					size = sprintf(at_message, NETIF_ATCMD_4G_MQTT_PUBLISH,
															qos,
															120,	// timeout
															retain,	// retain
															0);		//dup
					netif_core_4g_output(at_message, size);
					retry = 0;
					// Switch to wait for transmiting payload
					state = STATE_4G_MQTT_WAIT_FOR_RESPONSE;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G,true);
					if(retry >= NETIF_MAX_RETRY){
						utils_log_error("MQTT publish got error, maximum retried time\r\n");
						retry = 0;
						state = STATE_4G_MQTT_PUBLISH_TOPIC;
						return NETIF_FAIL;
					}
					utils_log_warn("MQTT publish got error, retrying ...\r\n");
					retry ++;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
				}
			}
			break;
		case STATE_4G_MQTT_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				netif_core_atcmd_reset(NETIF_4G,true);
				if(retry >= NETIF_MAX_RETRY){
					utils_log_error("MQTT wait for response timeout, maximum retried time\r\n");
					retry = 0;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_FAIL;
				}
				utils_log_warn("MQTT wait for response timeout, retrying ...\r\n");
				retry ++;
				state = STATE_4G_MQTT_PUBLISH_TOPIC;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G,&response)){
				if(response == NETIF_RESPONSE_OK){
					// Reset Buffer
					netif_core_atcmd_reset(NETIF_4G,true);
					// Switch to Publish Message
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR){
					netif_core_atcmd_reset(NETIF_4G,true);
					if(retry >= NETIF_MAX_RETRY){
						utils_log_error("MQTT wait for response got error, maximum retried time\r\n");
						retry = 0;
						state = STATE_4G_MQTT_PUBLISH_TOPIC;
						return NETIF_FAIL;
					}
					utils_log_warn("MQTT wait for response got error, retrying ...\r\n");
					retry ++;
					state = STATE_4G_MQTT_PUBLISH_TOPIC;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}
#endif


#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
/**
 * @brief Parse On Message
 * state
 * 0: Get LinkID	// Ignore
 * 1: Get Topic
 * 2: Get Payload len
 * 3: Get Payload
 * @return netif_status_t Status of Process
 */
static bool netif_wifi_ethernet_mqtt_parse_on_message(){
	static char payload_length_buffer[16];
	static uint8_t payload_length_index = 0;
	static uint8_t state = 0;
	static uint8_t start_get_topic = false;
	static uint8_t start_get_payload = false;
	uint8_t data;
	switch (state) {
		case 0:
			if(netif_core_atcmd_get_data_after(NETIF_WIFI_ETHERNET, &data)){
				if(data == ','){
					state = 1;
				}
			}
			break;
		case 1:
			if(netif_core_atcmd_get_data_after(NETIF_WIFI_ETHERNET, &data)){
				if(data == ','){
					state = 2;
				}else if(data != '"' ){
					topic[topic_len++] = data;
				}
			}
			break;
		case 2:
			if(netif_core_atcmd_get_data_after(NETIF_WIFI_ETHERNET, &data)){
				if(data == ','){
					state = 3;
				}
			}
			break;
		case 3:
			if(netif_core_atcmd_get_data_after(NETIF_WIFI_ETHERNET, &data)){
				if(data == '\n'){
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
					state = 0;
					return true;
				}else{
					payload[payload_len++] = data;
				}
			}
			break;
		default:
			break;
	}
	return false;
}
#endif

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
/*
 * Step Description
 * 1: Get Topic
 * 2: Get Payload
 */
static bool netif_4g_mqtt_parse_on_message(){
	static char * topic_pattern = "+CMQTTRXTOPIC:";
	static char * payload_pattern = "+CMQTTRXPAYLOAD:";
	static char * end_pattern = "+CMQTTRXEND:";
	static uint8_t state = 0;
	static uint16_t on_message_buffer_index = 0;
	uint8_t data;
	switch (state) {
		// Check Topic Pattern
		case 0:
			if(netif_core_atcmd_get_data_after(NETIF_4G,&on_message_buffer[on_message_buffer_index++])){
				// Check Pattern is match or not
				if(utils_string_is_receive_data(on_message_buffer, on_message_buffer_index, topic_pattern)){
					on_message_buffer_index = 0;
					state = 1;
				}
			}
			break;
		// Delete Suffix
		case 1:
			if(netif_core_atcmd_get_data_after(NETIF_4G,&data)){
				// If is EOL -> Start Get Topic
				if(data == '\n'){
					state = 2;
				}
			}
			break;
		case 2:
			if(netif_core_atcmd_get_data_after(NETIF_4G,&on_message_buffer[on_message_buffer_index++])){
				// Check Pattern is match or not
				if(utils_string_is_receive_data(on_message_buffer, on_message_buffer_index, payload_pattern)){
					// Get Topic
					topic_len = on_message_buffer_index - 2 - strlen(payload_pattern);
					memcpy(topic, on_message_buffer, topic_len);
					on_message_buffer_index = 0;
					state = 3;
				}
			}
			break;
		// Delete Suffix
		case 3:
			if(netif_core_atcmd_get_data_after(NETIF_4G,&data)){
				// If is EOL -> Start Get Topic
				if(data == '\n'){
					state = 4;
				}
			}
			break;
		case 4:
			if(netif_core_atcmd_get_data_after(NETIF_4G,&on_message_buffer[on_message_buffer_index++])){
				// Check Pattern is match or not
				if(utils_string_is_receive_data(on_message_buffer, on_message_buffer_index, end_pattern)){
					// Get Payload
					payload_len = on_message_buffer_index - 2 - strlen(end_pattern);
					memcpy(payload, on_message_buffer, payload_len);
					on_message_buffer_index = 0;
					state = 0;
					return true;
				}
			}
			break;
		default:
			break;
	}
	return false;
}

#endif
