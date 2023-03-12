#include "manager/netif_4g.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_4g.h"
#include "utils/utils_logger.h"



// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Imei
static char imei[NETIF_4G_IMEI_SIZE];
// Setting Command
static char * setting_command[] = {
    NETIF_ATCMD_4G_ECHO_ENABLE,
    NETIF_ATCMD_4G_PDP_CONTEXT,
    NETIF_ATCMD_4G_SOCKET_CONFIG,
    NETIF_ATCMD_4G_DISABLE_TCP_MODE
};
// Connection status
static bool _4g_connected = false;

/************************************ Internal Functions********************************/
// 4G Request State
enum {
	STATE_4G_IDLE,
	STATE_4G_STARTUP,
	STATE_4G_SETTING,
	STATE_4G_GET_IMEI,
	STATE_4G_WAIT_FOR_RESPONSE
};

enum {
	STATE_4G_STARTUP_RESET_ENA,
	STATE_4G_STARTUP_RESET_DIS,
	STATE_4G_STARTUP_PWRON_DIS,
	STATE_4G_STARTUP_PWRON_ENA,
	STATE_4G_STARTUP_WAIT_FOR_RESPONSE,
};

// Handle imei response
static netif_status_t netif_4g_startup();
static netif_status_t netif_4g_setting();
static bool netif_4g_handle_imei_response(uint8_t *data, uint16_t data_size, uint8_t *imei);

/**
 * @brief Initialize 4G 
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_init(){
	return NETIF_OK;
}



/**
 * @brief Run 4G in Supper Loop
 *
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_run(){
	// Startup 4G Module
	static uint8_t state = STATE_4G_STARTUP;
	netif_status_t ret;
	switch (state) {
	case STATE_4G_STARTUP:
		ret = netif_4g_startup();
		if(ret == NETIF_OK){
			state = STATE_4G_SETTING;
		}else if(ret != NETIF_IN_PROCESS){
			return ret;
		}
		break;
	case STATE_4G_SETTING:
		ret = netif_4g_setting();
		if(ret == NETIF_OK){
			state = STATE_4G_IDLE;
			return NETIF_OK;
		}else if(ret != NETIF_IN_PROCESS){
			state = STATE_4G_STARTUP;
			return ret;
		}
		break;
	case STATE_4G_IDLE:
		return NETIF_OK;
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

/**
 * @brief Deinitialize 4G
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_deinit(){
    //TODO: 
    // Stop Module
    return NETIF_OK;
}

/**
 * @brief Get IMEI of 4G Module
 * 
 * @param imei Pointer to Imei String Buffer
 * @param imei_max_size Max Size Imei String Buffer
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_get_imei(char * imei, size_t imei_max_size){
    static uint8_t step = STATE_4G_GET_IMEI;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	uint8_t *data;
	size_t data_size;
	int size;
	switch (step) {
		case STATE_4G_GET_IMEI:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(true);
                size = sprintf(at_message, NETIF_ATCMD_4G_GET_IMEI);
				netif_core_4g_output(at_message, size);
				step = STATE_4G_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_WAIT_FOR_RESPONSE:
			// Check Timeout
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
				// Reset State
				step = STATE_4G_GET_IMEI;
				// Return TIMEOUT
				return NETIF_TIMEOUT;

			}
			if(netif_core_atcmd_is_responded(&response)){
                // Get Data
				netif_core_atcmd_get_data_before(&data, &data_size);
				if(response == NETIF_RESPONSE_OK){
                    // Handle Data response
                    if(netif_4g_handle_imei_response(data,data_size,imei)){
                        utils_log_info("Imei: %s\r\n", imei);
                    }else{
                        utils_log_error("Get imei error\r\n");
                    }
					netif_core_atcmd_reset(true);
					retry = 0;
					step = STATE_4G_GET_IMEI;
					return NETIF_OK;
				}
				else if(response == NETIF_RESPONSE_ERROR
						 || response == NETIF_WIFI_ETHERNET_REPORT_BUSY){
					if(retry >= NETIF_MAX_RETRY){
						netif_core_atcmd_reset(false);
						retry = 0;
						step = STATE_4G_GET_IMEI;
						return NETIF_FAIL;
					}
					retry ++;
					step = STATE_4G_GET_IMEI;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

/**
 * @brief Send Message to Other Phone
 * 
 * @param phone_number Phone Number of Target Sim 
 * @param message Message Sent
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_send_sms(char * phone_number, char * message){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Get 4G Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_connected(bool *connected){
    //TODO: 
	*connected = _4g_connected;
    return NETIF_OK;
}

/**
 * @brief Get 4G Internet Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_internet_connected(bool *connected){
    //TODO: 
    return NETIF_OK;
}



static netif_status_t netif_4g_startup(){
	// Startup 4G Module
	static uint8_t state = STATE_4G_STARTUP;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (state) {
	case STATE_4G_STARTUP_RESET_ENA:
		utils_log_debug("STATE_4G_STARTUP_RESET_ENA\r\n");
		NETIF_4G_RESET(true);
		last_time_sent = NETIF_GET_TIME_MS();
		state = STATE_4G_STARTUP_RESET_DIS;
		break;
	case STATE_4G_STARTUP_RESET_DIS:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_RESET_DURATION){
			utils_log_debug("STATE_4G_STARTUP_RESET_DIS\r\n");
			NETIF_4G_RESET(false);
			last_time_sent = NETIF_GET_TIME_MS();
			state = STATE_4G_STARTUP_PWRON_DIS;
		}
		break;
	case STATE_4G_STARTUP_PWRON_DIS:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_DELAY_BETWEEN_RESETANDPWRON){
			utils_log_debug("STATE_4G_STARTUP_PWRON_DIS\r\n");
			NETIF_4G_POWER(false);
			last_time_sent = NETIF_GET_TIME_MS();
			state = STATE_4G_STARTUP_PWRON_ENA;
		}
		break;
	case STATE_4G_STARTUP_PWRON_ENA:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_POWER_DURATION){
			utils_log_debug("STATE_4G_STARTUP_PWRON_ENA\r\n");
			NETIF_4G_POWER(true);
			last_time_sent = NETIF_GET_TIME_MS();
			state = STATE_4G_STARTUP_WAIT_FOR_RESPONSE;
		}
	case STATE_4G_STARTUP_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_WAIT_FOR_STARTUP_DURATION){
			utils_log_debug("STATE_4G_STARTUP_WAIT_FOR_RESPONSE\r\n");
			// Reset State
			state = STATE_4G_STARTUP_RESET_ENA;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(&response)){
			if(response == NETIF_4G_REPORT_INITIALIZE_DONE){
				utils_log_debug("NETIF_4G_REPORT_INITIALIZE_DONE\r\n");
				_4g_connected = true;
				netif_core_atcmd_reset(true);
				state = STATE_4G_STARTUP_RESET_ENA;
				return NETIF_OK;
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}


static netif_status_t netif_4g_setting(){
	// Setting 4G module
	static size_t nb_setting_command = sizeof(setting_command) / sizeof(setting_command[0]);
	static uint8_t step = STATE_4G_SETTING;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	static uint8_t current_command_idx = 0;
	netif_core_response_t response;
	int size;
	switch (step) {
	case STATE_4G_SETTING:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
			last_time_sent = NETIF_GET_TIME_MS();
			// Clear Before Data
			netif_core_atcmd_reset(true);
			size = sprintf(at_message, setting_command[current_command_idx++]);
			utils_log_debug(at_message);
			netif_core_4g_output(at_message, size);
			step = STATE_4G_WAIT_FOR_RESPONSE;
		}
		break;
	case STATE_4G_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			utils_log_debug("Timeout\r\n");
			// Reset State
			step = STATE_4G_SETTING;
			retry = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(&response)){
			if(response == NETIF_RESPONSE_OK){
				netif_core_atcmd_reset(true);
				retry = 0;
				step = STATE_4G_SETTING;
				if(current_command_idx >= nb_setting_command){
					// Reset Setting Command Idx
					current_command_idx = 0;
					return NETIF_OK;
				}
			}
			else if(response == NETIF_RESPONSE_ERROR){
				if(retry >= NETIF_MAX_RETRY){
					netif_core_atcmd_reset(false);
					retry = 0;
					step = STATE_4G_SETTING;
					return NETIF_FAIL;
				}
				retry ++;
				step = STATE_4G_SETTING;
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

static bool netif_4g_handle_imei_response(uint8_t *data, uint16_t data_size, uint8_t *imei){
    static char imei_prefix[] = "+SIMEI:";
    static size_t imei_prefix_len = sizeof(imei_prefix);
    // Get prefix
    char * data_p = strnstr(data, imei_prefix, data_size);
    // Check it's found or not
    if(data_p){
        // Get imei
        memcpy(imei, &data_p[imei_prefix_len], NETIF_4G_IMEI_SIZE);
        return true;
    }
    return false;
}
