#include "core/netif_core.h"
#include "netif_def.h"
#include "utils/utils_buffer.h"
#include "utils/utils_logger.h"
#include "utils/utils_string.h"

static const char * at_response_table[] = {
    [NETIF_RESPONSE_OK] = "OK",
    [NETIF_RESPONSE_ERROR] = "ERROR",
    [NETIF_RESPONSE_INPUT] = ">",
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
    [NETIF_WIFI_ETHERNET_RESPONSE_SEND_OK] = "SEND OK",
    [NETIF_WIFI_ETHERNET_RESPONSE_SEND_FAIL] = "SEND FAIL",
    [NETIF_WIFI_ETHERNET_RESPONSE_SET_OK] = "SET OK",
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
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_MESSAGE_OK] = "+MQTTSUBRECV:",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_OK] = "+MQTTPUB:OK",
    [NETIF_WIFI_ETHERNET_REPORT_MQTT_PUB_FAIL] = "+MQTTPUB:FAIL",
#endif
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
	[NETIF_4G_REPORT_SMS_DONE] = "SMS DONE",
    [NETIF_4G_REPORT_INITIALIZE_DONE] = "PB DONE",
    [NETIF_4G_REPORT_MQTT_CONNECTED] = "+CMQTTCONNECT: 0,0",        // client index is 0, err is 0
    [NETIF_4G_REPORT_MQTT_DISCONNECTED] = "+CMQTTCONNLOST: 0",
    [NETIF_4G_REPORT_MQTT_MESSAGE_OK] = "+CMQTTRXSTART: 0",
    [NETIF_4G_REPORT_MQTT_PUB_OK] = "+CMQTTPUB: 0,0",
#endif

};
static uint16_t at_response_table_size = sizeof(at_response_table)/sizeof(at_response_table[0]);


#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
static utils_buffer_t buffer_wifi_ethernet;
// Core buffer for Wifi-Ethernet Module
static uint8_t core_wifi_ethernet_buffer[BUFFER_MAX_SIZE];
static uint16_t core_wifi_ethernet_buffer_index = 0;
static bool at_response_wifi_ethernet_indication = false;
static netif_core_response_t at_response_wifi_ethernet;
#endif

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
// Netif Buffer to receive response from Module
static utils_buffer_t buffer_4g;
// Core buffer for 4G Module
static uint8_t core_4g_buffer[BUFFER_MAX_SIZE];
static uint16_t core_4g_buffer_index = 0;
static bool at_response_4g_indication = false;
// AT Indication
static netif_core_response_t at_response_4g;
#endif


// Internal Function
static void netif_core_process_response(){
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
	// If at_response is'not reset -> Ignore
	if(!at_response_wifi_ethernet_indication){
		if(utils_buffer_is_available(&buffer_wifi_ethernet)){
			volatile uint8_t * data_p = &core_wifi_ethernet_buffer[core_wifi_ethernet_buffer_index];
			core_wifi_ethernet_buffer_index = (core_wifi_ethernet_buffer_index+1) % BUFFER_MAX_SIZE;
			utils_buffer_pop(&buffer_wifi_ethernet,data_p);
			// Check Response buffer match with any in at reponse table
			for (size_t i = 0; i < at_response_table_size; i++)
			{
				if (utils_string_is_receive_data(core_wifi_ethernet_buffer,
													core_wifi_ethernet_buffer_index,
													at_response_table[i])){
					// Match with index i
					at_response_wifi_ethernet = (netif_core_response_t)i;
	            	utils_log_debug("%s\r\n", at_response_table[i]);
					at_response_wifi_ethernet_indication = true;
					break;
				}
			}
		}
	}
#endif

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
	if(!at_response_4g_indication){
		if(utils_buffer_is_available(&buffer_4g)){
			volatile uint8_t * data_p = &core_4g_buffer[core_4g_buffer_index];
			core_4g_buffer_index = (core_4g_buffer_index+1) % BUFFER_MAX_SIZE;
			utils_buffer_pop(&buffer_4g,data_p);
			// Check Response buffer match with any in at reponse table
			for (size_t i = 0; i < at_response_table_size; i++)
			{
				if (utils_string_is_receive_data(core_4g_buffer,
													core_4g_buffer_index,
													at_response_table[i])){
					// Match with index i
					at_response_4g = (netif_core_response_t)i;
	            	utils_log_debug("%s\r\n", at_response_table[i]);
					at_response_4g_indication = true;
					break;
				}
			}
		}
	}
#endif
}

// Generic Function
/**
 * @brief Initialize Network Interface Core Stack
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
netif_status_t netif_core_init(){
	utils_log_debug("Netif Core Init\r\n");
    // Cleanup Buffer
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
    utils_buffer_init(&buffer_4g, sizeof(uint8_t));
#endif
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
    utils_buffer_init(&buffer_wifi_ethernet, sizeof(uint8_t));
#endif
    return NETIF_OK;
}

/**
 * @brief Run Network Interface Core Stack in Supper Loop
 * 
 * @return true If OK
 * @return false If failed
 */
netif_status_t netif_core_run(){
    uint8_t data;
    // Process Reponse from Module (4G or Wifi_Ethernet)
    netif_core_process_response();
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
    // Check Input from 4G Module
    if(NETIF_4G_INPUT_IS_AVAILABLE()){
        data = NETIF_4G_INPUT();
        utils_buffer_push(&buffer_4g,&data);
    }
#endif

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
    // Check Input from Wifi/Ethernet Module
    if(NETIF_WIFI_ETHERNET_INPUT_IS_AVAILABLE()){
        data = NETIF_WIFI_ETHERNET_INPUT();
//        utils_log_info("%c", data);
        utils_buffer_push(&buffer_wifi_ethernet,&data);
    }
#endif
    return NETIF_OK;
}

/**
 * @brief Deinitialize Network Interface Core Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_core_deinit(){
	utils_log_info("Netif Core Deinit\r\n");
    // Do something
    return NETIF_OK;
}

/**
 * @brief Check ATCMD is have response match with atcommand table
 * 
 * @param response Pointer to response
 * @return true if have response
 * @return false if not
 */
bool netif_core_atcmd_is_responded(netif_module_t module, netif_core_response_t* response){
	switch (module) {
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G:
			if(at_response_4g_indication){
				*response = at_response_4g;
				return true;
			}
			break;
#endif
#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_WIFI_ETHERNET:
			if(at_response_wifi_ethernet_indication){
				*response = at_response_wifi_ethernet;
				return true;
			}
			break;
#endif
		default:
			break;
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
bool netif_core_atcmd_get_data_before(netif_module_t module, uint8_t **data, size_t * data_size){
	switch (module) {
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G:
			if(at_response_4g_indication){
				*data = core_4g_buffer;
				*data_size = core_4g_buffer_index;
				return true;
			}
			break;
#endif

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_WIFI_ETHERNET:
			if(at_response_wifi_ethernet_indication){
				*data = core_wifi_ethernet_buffer;
				*data_size = core_wifi_ethernet_buffer_index;
				return true;
			}
#endif
		default:
			break;
	}
    return false;
}


/**
 * @brief Get Data after the AT Reponse
 *
 * @param data Pointer to data*, Get buffer Pointer of Core Buffer
 * @param data_size Get Size
 * @return true if OK
 * @return false if failed
 */
bool netif_core_atcmd_get_data_after(netif_module_t module, uint8_t *data){
	switch (module) {
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G:
			if(at_response_4g_indication){
				if(utils_buffer_is_available(&buffer_4g)){
					utils_buffer_pop(&buffer_4g,data);
					return true;
				}
			}
			break;
#endif

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_WIFI_ETHERNET:
			if(at_response_wifi_ethernet_indication){
				if(utils_buffer_is_available(&buffer_wifi_ethernet)){
					utils_buffer_pop(&buffer_wifi_ethernet,data);
					return true;
				}
			}
			break;
#endif
		default:
			break;
	}

	return false;
}

/**
 * @brief Reset AT Command Buffer and Response Result
 * 
 * @return true if Ok
 * @return false if false
 */
bool netif_core_atcmd_reset(netif_module_t module,  bool reset_buffer){
	switch (module) {
#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_4G:
			if(reset_buffer){
				core_4g_buffer_index = 0;
				memset(core_4g_buffer,0,BUFFER_MAX_SIZE);
			}
			at_response_4g_indication = false;
			break;
#endif

#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_WIFI_ETHERNET:
			if(reset_buffer){
				core_wifi_ethernet_buffer_index = 0;
				memset(core_wifi_ethernet_buffer,0,BUFFER_MAX_SIZE);
			}
			at_response_wifi_ethernet_indication = false;
			break;
		default:
#endif
			break;
	}
    return true;
}

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
/**
 * @brief Input data from 4G Module to Netif Core
 * 
 * @param data Pointer of data
 * @param data_size Data size
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_core_4g_input(uint8_t* data, size_t data_size){
	for (int var = 0; var < data_size; ++var) {
		utils_buffer_push(&buffer_4g, &data[var]);
	}
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
#endif


#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
/**
 * @brief Input data from Wifi-Ethernet Device to Netif Core Stack
 * 
 * @param data Pointer to data
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed or Timeout
 */
bool netif_core_wifi_ethernet_input(uint8_t* data, size_t data_size){
	for (int var = 0; var < data_size; ++var) {
		utils_buffer_push(&buffer_wifi_ethernet, &data[var]);
	}
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
    utils_log_debug("%.*s", data_size, (char*)data);
    NETIF_WIFI_ETHERNET_OUTPUT(data, data_size);
}
#endif




