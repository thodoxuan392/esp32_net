#include "stdbool.h"
#include "esp32_net.h"
#include "at_command/at_command.h"
#include "utils/esp32_net_logger.h"

static void ESP32NET_state_machine();
static void ESP32NET_wifi_connected();
static void ESP32NET_wifi_waiting_smartconfig();
static void ESP32NET_ethernet_connected();
static void ESP32NET_disconnected();
static void ESP32NET_at_process();
static void ESP32NET_at_cmd_rsp_callback(at_cmd_rsp_t at_response);
static bool ESP32NET_at_cmd_rsp_available();
static void ESP32NET_at_cmd_rsp_push(at_cmd_rsp_t at_response);
static at_cmd_rsp_t ESP32NET_at_cmd_rsp_pop();
static void ESP32NET_send_at_request(at_cmd_req_t);
static void ESP32NET_reset_request();
static void ESP32NET_reset_response();

static esp32_state_t esp32net_state = ESP32NET_DISCONNECTED;

// AT Command Response Raw
static uint8_t at_buffer[ESP32_NET_AT_BUFFER_LEN];
static uint16_t at_buffer_tail = 0;
static uint16_t at_buffer_head = 0;

// AT Command Response Buffer
static at_cmd_rsp_t at_cmd_rsp_buffer[ESP32_NET_AT_CMD_RSP_LEN];
static uint16_t at_cmd_rsp_buffer_tail = 0;
static uint16_t at_cmd_rsp_buffer_head = 0;

// AT Command Request
static at_cmd_req_t at_cmd_req = {
		.id = AT_CMD_ID_UNKNOWN
};
// AT Command Response
static at_cmd_rsp_t at_cmd_rsp = {
		.id = AT_CMD_ID_UNKNOWN
};

/**
 * @brief Init ESP32 Network Stack
 *
 * @param handle Handler
 * @param config Configuration for esp32net
 */
void esp32_netmanager_init(){
	// Some initialization
}

/**
 * @brief Put data to ESP32_Net to stack
 *
 * @param handle Handler
 * @param data Pointer to input data
 * @param data_len Input data length
 */
void esp32_netmanager_input(uint8_t * data, size_t data_len){
	memcpy(&at_buffer[at_buffer_head], data, data_len);
	at_buffer_head = (at_buffer_head + data_len)%ESP32_NET_AT_BUFFER_LEN;
}

/**
 * @brief Output Callback from Stack
 *
 * @param handle Handler
 * @param data Pointer to output data
 * @param data_len Output datalen
 */
__attribute__((weak)) void esp32_netmanager_output(uint8_t *data, size_t data_len){
	(void)data;
	(void)data_len;
}


/**
 * @brief Loop process for data processing, it take 2-3ms in super loop
 * - Support Mqtt
 * - Support Http
 * @param handle Handler
 */
void esp32_netmanager_loop(){
	ESP32NET_state_machine();
	ESP32NET_at_process();
}

static void ESP32NET_state_machine(){
	switch (esp32net_state) {
		case ESP32NET_DISCONNECTED:
			ESP32NET_disconnected();
			break;
		case ESP32NET_WIFI_WAIT_SMARTCONFIG:
			ESP32NET_wifi_waiting_smartconfig();
			break;
		case ESP32NET_WIFI_CONNECTED:
			ESP32NET_wifi_connected();
			break;
		case ESP32NET_ETHERNET_CONNECTED:
			ESP32NET_ethernet_connected();
			break;
		default:
			break;
	}
}

/**
 * 1. Check Ethernet Connection -> If have, switch to Ethernet Connected Mode
 * 2. Check Previous Wifi Connect -> If have, try to Connect to wifi- -> If Ok switch to WIFI Connected Mode, if timeout -> Enable Smartconfig
 * 3. Waiting in SmartConfig Mode -> If have wifi config -> Connect to wifi -> Switch to Wifi connected Mode
 * 4. In Connected Mode -> Check if Ethernet or Wifi Disconnect -> Switch to Disconnected Mode
 */


/*
 * disconnect_action
 * 0: Check Ethernet Connection
 * 1: Wait for Ethernet Connection Response
 * 2: Wait for Wifi Connection Before
 * 3: Switch to Waiting SmartConfig
 */
static void ESP32NET_disconnected(){
	static uint8_t disconnect_action = 0;
	at_cmd_req_t request;
	at_cmd_rsp_t response;
	switch (disconnect_action) {
		case 0:
			// Check Ethernet Connection -> Send AT request check Ethernet Connection
			ESP32NET_log_info("Check Ethernet Connection");
			request.id = AT_CMD_ID_BASIC_TEST;
			request.ops = AT_CMD_OPS_EXEC;
			ESP32NET_send_at_request(request);
			disconnect_action = 1;
			break;
		case 1:
			// Check Ethernet Connection -> Send AT request check Ethernet Connection
			if(ESP32NET_at_cmd_rsp_available()){
				response = ESP32NET_at_cmd_rsp_pop();
				if(response.result == AT_CMD_RESULT_OK){
					// Switch to Ethernet Connected Mode
					ESP32NET_log_debug("AT_CMD_RESULT_OK");
					esp32net_state = ESP32NET_ETHERNET_CONNECTED;
					ESP32NET_log_debug("SWITCH TO ESP32NET_ETHERNET_CONNECTED");
				}else{
					// Connect to Wifi Before
					request.id = AT_CMD_ID_BASIC_TEST;
					request.ops = AT_CMD_OPS_EXEC;
					ESP32NET_send_at_request(request);
					disconnect_action = 2;
				}
			}
			break;
		case 2:
			// Check Ethernet Connection -> Send AT request check Ethernet Connection
			if(ESP32NET_at_cmd_rsp_available()){
				response = ESP32NET_at_cmd_rsp_pop();
				if(response.result == AT_CMD_RESULT_OK){
					// Switch to Ethernet Connected Mode
					ESP32NET_log_debug("AT_CMD_RESULT_OK");
					esp32net_state = ESP32NET_WIFI_CONNECTED;
					ESP32NET_log_debug("SWITCH TO ESP32NET_WIFI_CONNECTED");
				}else{
					// Switch to SmartConfig Mode
					ESP32NET_log_debug("SWITCH TO ESP32NET_WIFI_WAIT_SMARTCONFIG");
					esp32net_state = ESP32NET_WIFI_WAIT_SMARTCONFIG;
					// Reset disconnect_action
					disconnect_action = 0;
				}
			}
			break;
		default:
			break;
	}
}

static void ESP32NET_wifi_connected(){
	static uint8_t first_wifi_connected = 0;
	if(!first_wifi_connected){
		// Do first enter to Ethernet Connected
		ESP32NET_log_debug("Do first enter to WIFI CONNECTED");
		first_wifi_connected = 1;
	}else{
		ESP32NET_log_debug("Do enter to WIFI CONNECTED");
	}
}

static void ESP32NET_wifi_waiting_smartconfig(){
	static uint8_t first_wifi_waiting_smartconfig = 0;
	if(!first_wifi_waiting_smartconfig){
		// Do first enter to Ethernet Connected
		ESP32NET_log_debug("Do first enter to WAITING SMART CONFIG");
		first_wifi_waiting_smartconfig = 1;
	}else{
		ESP32NET_log_debug("Do enter to Ethernet Connected");
	}
}


static void ESP32NET_ethernet_connected(){
	static uint8_t first_ethernet_connected = 0;
	if(!first_ethernet_connected){
		// Do first enter to Ethernet Connected
		ESP32NET_log_debug("Do first enter to Ethernet Connected");
		first_ethernet_connected = 1;
	}else{
		ESP32NET_log_debug("Do enter to Ethernet Connected");
	}
}

static void ESP32NET_at_process(){
	at_cmd_rsp_t at_cmd_rsp;
	uint8_t * response = at_buffer + at_buffer_tail;
	uint16_t  response_len = at_buffer_head - at_buffer_tail;
	// AT Command Parse from Response
	at_cmd_rsp = at_command_parse_response(at_cmd_req.id , response, response_len);
	if(at_cmd_rsp.id != AT_CMD_ID_UNKNOWN){
		ESP32NET_at_cmd_rsp_callback(at_cmd_rsp);
		// Reset ID of at_cmd_rsp
		ESP32NET_reset_request();
	}
}

static void ESP32NET_at_cmd_rsp_callback(at_cmd_rsp_t at_response){
	ESP32NET_at_cmd_rsp_push(at_response);
}


static bool ESP32NET_at_cmd_rsp_available(){
	return at_cmd_rsp_buffer_head != at_cmd_rsp_buffer_tail;
}

static void ESP32NET_at_cmd_rsp_push(at_cmd_rsp_t at_response){
	at_cmd_rsp_buffer[at_cmd_rsp_buffer_head] = at_response;
	at_cmd_rsp_buffer_head = (at_cmd_rsp_buffer_head + 1) % ESP32_NET_AT_CMD_RSP_LEN;
}


static at_cmd_rsp_t ESP32NET_at_cmd_rsp_pop(){
	at_cmd_rsp_t temp = at_cmd_rsp_buffer[at_cmd_rsp_buffer_tail];
	at_cmd_rsp_buffer_tail = (at_cmd_rsp_buffer_tail + 1) % ESP32_NET_AT_CMD_RSP_LEN;
	return temp;
}

static void ESP32NET_send_at_request(at_cmd_req_t request){
	at_cmd_req = request;
	char *at_str = at_command_parse_request(request);
	esp32_netmanager_output(at_str, strlen(at_str));
}

static void ESP32NET_reset_request(){
	at_cmd_req.id = AT_CMD_ID_UNKNOWN;
}
static void ESP32NET_reset_response(){
	at_cmd_rsp.id = AT_CMD_ID_UNKNOWN;
}

