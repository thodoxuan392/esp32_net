#include "manager/netif_4g.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_4g.h"
#include "utils_logger.h"

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1

#define NETIF_4G_IMSI_MAX_LENGTH 20
#define NETIF_4G_ICCID_MAX_LENGTH 20

// 4G main State
enum {
	STATE_4G_STARTUP,
	STATE_4G_SETTING,
	STATE_4G_IMSI,
	STATE_4G_ICCID,
	STATE_4G_IDLE
};

// 4G startup state
enum {
	STATE_4G_STARTUP_RESET_ENA,
	STATE_4G_STARTUP_RESET_DIS,
	STATE_4G_STARTUP_PWRON_DIS,
	STATE_4G_STARTUP_PWRON_ENA,
	STATE_4G_STARTUP_WAIT_FOR_RESPONSE,
	STATE_4G_STARTUP_SOFTWARE_RESET,
	STATE_4G_STARTUP_WAIT_FOR_SOFTWARE_RESET,
};

// 4G setting state
enum {
	STATE_4G_SETTING_SEND_COMMAND,
	STATE_4G_SETTING_WAIT_FOR_RESPONSE
};

// 4G imsi state
enum {
	STATE_4G_IMSI_SEND_COMMAND,
	STATE_4G_IMSI_WAIT_FOR_RESPONSE
};

// 4G iccid state
enum {
	STATE_4G_ICCID_SEND_COMMAND,
	STATE_4G_ICCID_WAIT_FOR_RESPONSE
};


// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Setting Command
static char * setting_command[] = {
    NETIF_ATCMD_4G_ECHO_DISABLE,
    NETIF_ATCMD_4G_PDP_CONTEXT,
    NETIF_ATCMD_4G_SOCKET_CONFIG,
    NETIF_ATCMD_4G_NON_TRANSPARENT_TCP_MODE,
	NETIF_ATCMD_4G_TCP_RX_POLLING_MODE,
};


// Main state
static uint8_t main_state = STATE_4G_STARTUP;

// Startup
static uint8_t startup_state = STATE_4G_STARTUP_RESET_ENA;

// Setting
static uint8_t setting_state = STATE_4G_SETTING_SEND_COMMAND;
static uint8_t setting_retry = 0;
static uint8_t setting_index = 0;

// Connection status
static bool _4g_connected = false;

// Imsi
static uint8_t _4g_imsi_state = STATE_4G_IMSI_SEND_COMMAND;
static uint8_t _4g_imsi[NETIF_4G_IMSI_MAX_LENGTH];
static uint32_t _4g_imsi_length = 0;
static bool _4g_imsi_completed = false;
static uint8_t _4g_imsi_retry = 0;

// Imsi
static uint8_t _4g_iccid_state = STATE_4G_IMSI_SEND_COMMAND;
static uint8_t _4g_iccid[NETIF_4G_IMSI_MAX_LENGTH];
static uint32_t _4g_iccid_length = 0;
static bool _4g_iccid_completed = false;
static uint8_t _4g_iccid_retry = 0;

/************************************ Internal Functions********************************/



// Handle imei response
static netif_status_t netif_4g_startup();
static netif_status_t netif_4g_setting();
static netif_status_t netif_4g_imsi();
static netif_status_t netif_4g_iccid();
static netif_status_t netif_4g_idle();

// Parser
static netif_status_t netif_4g_parse_on_imsi(uint8_t* imsi, uint32_t *imsiSize);
static netif_status_t netif_4g_parse_on_iccid(uint8_t* iccid, uint32_t *iccidSize);


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
	netif_status_t ret;
	switch (main_state) {
	case STATE_4G_STARTUP:
		ret = netif_4g_startup();
		if(ret == NETIF_OK){
			main_state = STATE_4G_SETTING;
			utils_log_info("4G startup OK\r\n");
		}else if(ret != NETIF_IN_PROCESS){
			utils_log_info("4G startup Failed\r\n");
			return ret;
		}
		break;
	case STATE_4G_SETTING:
		ret = netif_4g_setting();
		if(ret == NETIF_OK){
			main_state = STATE_4G_IMSI;
			utils_log_info("4G setting OK\r\n");
			return NETIF_OK;
		}else if(ret != NETIF_IN_PROCESS){
			utils_log_info("4G setting Failed\r\n");
			main_state = STATE_4G_STARTUP;
			return ret;
		}
		break;
	case STATE_4G_IMSI:
		ret = netif_4g_imsi();
		if(ret == NETIF_OK){
			main_state = STATE_4G_ICCID;
			utils_log_info("4G Imsi OK\r\n");
			return NETIF_OK;
		}else if(ret != NETIF_IN_PROCESS){
			utils_log_info("4G Imsi Failed\r\n");
			main_state = STATE_4G_STARTUP;
			return ret;
		}
		break;
	case STATE_4G_ICCID:
		ret = netif_4g_iccid();
		if(ret == NETIF_OK){
			main_state = STATE_4G_IDLE;
			utils_log_info("4G Iccid OK\r\n");
			return NETIF_OK;
		}else if(ret != NETIF_IN_PROCESS){
			utils_log_info("4G Iccid Failed\r\n");
			main_state = STATE_4G_STARTUP;
			return ret;
		}
		break;
	case STATE_4G_IDLE:
		ret = netif_4g_idle();
		if(ret != NETIF_OK){
			main_state = STATE_4G_STARTUP;
		}
		return NETIF_OK;
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

netif_status_t netif_4g_reset(){
	main_state = STATE_4G_STARTUP;
	startup_state = STATE_4G_STARTUP_RESET_ENA;
	setting_state = STATE_4G_SETTING_SEND_COMMAND;
	setting_index = 0;
	setting_retry = 0;
	_4g_connected = false;
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
 * @brief Get 4G Isms
 */
netif_status_t netif_4g_get_imsi(uint8_t *imsi, uint32_t* imsiSize){
	if(!_4g_imsi_completed){
		return NETIF_FAIL;
	}
	strncpy(imsi, _4g_imsi, _4g_imsi_length);
	*imsiSize = _4g_imsi_length;
	return NETIF_OK;
}

/**
 * @brief Get 4G Iccid
 */
netif_status_t netif_4g_get_iccid(uint8_t *iccid, uint32_t* iccidSize){
	if(!_4g_iccid_completed){
		return NETIF_FAIL;
	}
	strncpy(iccid, _4g_iccid, _4g_iccid_length);
	*iccidSize = _4g_iccid_length;
	return NETIF_OK;
}

static netif_status_t netif_4g_startup(){
	// Startup 4G Module
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch (startup_state) {
	case STATE_4G_STARTUP_RESET_ENA:
		utils_log_debug("STATE_4G_STARTUP_RESET_ENA\r\n");
		NETIF_4G_RESET(true);
		last_time_sent = NETIF_GET_TIME_MS();
		startup_state = STATE_4G_STARTUP_RESET_DIS;
		break;
	case STATE_4G_STARTUP_RESET_DIS:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_RESET_DURATION){
			utils_log_debug("STATE_4G_STARTUP_RESET_DIS\r\n");
			NETIF_4G_RESET(false);
			last_time_sent = NETIF_GET_TIME_MS();
			startup_state = STATE_4G_STARTUP_PWRON_DIS;
		}
		break;
	case STATE_4G_STARTUP_PWRON_DIS:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_DELAY_BETWEEN_RESETANDPWRON){
			utils_log_debug("STATE_4G_STARTUP_PWRON_DIS\r\n");
			NETIF_4G_POWER(false);
			last_time_sent = NETIF_GET_TIME_MS();
			startup_state = STATE_4G_STARTUP_PWRON_ENA;
		}
		break;
	case STATE_4G_STARTUP_PWRON_ENA:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_POWER_DURATION){
			utils_log_debug("STATE_4G_STARTUP_PWRON_ENA\r\n");
			NETIF_4G_POWER(true);
			last_time_sent = NETIF_GET_TIME_MS();
			startup_state = STATE_4G_STARTUP_WAIT_FOR_RESPONSE;
		}
	case STATE_4G_STARTUP_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_WAIT_FOR_STARTUP_DURATION){
			utils_log_error("STATE_4G_STARTUP_WAIT_FOR_RESPONSE TIMEOUT\r\n");
			// Switch to Software Reset
			startup_state = STATE_4G_STARTUP_SOFTWARE_RESET;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
			if(response == NETIF_4G_REPORT_INITIALIZE_DONE){
				utils_log_debug("NETIF_4G_REPORT_INITIALIZE_DONE\r\n");
				netif_core_atcmd_reset(NETIF_4G, true);
				startup_state = STATE_4G_STARTUP_RESET_ENA;
				return NETIF_OK;
			}else {
				netif_core_atcmd_reset(NETIF_4G, false);
			}
		}
		break;
	case STATE_4G_STARTUP_SOFTWARE_RESET:
		last_time_sent = NETIF_GET_TIME_MS();
		netif_core_atcmd_reset(NETIF_4G, true);
		size = sprintf(at_message, NETIF_ATCMD_4G_RESET);
		utils_log_debug(at_message);
		netif_core_4g_output(at_message, size);
		startup_state = STATE_4G_STARTUP_WAIT_FOR_SOFTWARE_RESET;
		break;
	case STATE_4G_STARTUP_WAIT_FOR_SOFTWARE_RESET:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_4G_WAIT_FOR_SOFTWARE_RESET_DURATION){
			utils_log_error("STATE_4G_STARTUP_WAIT_FOR_SOFTWARE_RESET Timeout\r\n");
			// Reset State
			startup_state = STATE_4G_STARTUP_RESET_ENA;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
			if(response == NETIF_4G_REPORT_INITIALIZE_DONE){
				utils_log_info("STATE_4G_STARTUP_WAIT_FOR_SOFTWARE_RESET Ok\r\n");
				netif_core_atcmd_reset(NETIF_4G, true);
				startup_state = STATE_4G_STARTUP_RESET_ENA;
				return NETIF_OK;
			}else{
				// Clear other AT response
				netif_core_atcmd_reset(NETIF_4G, false);
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}


static netif_status_t netif_4g_setting(){
	static uint32_t last_time_sent = 0;
	// Setting 4G module
	size_t nb_setting_command = sizeof(setting_command) / sizeof(setting_command[0]);
	netif_core_response_t response;
	int size;
	switch (setting_state) {
	case STATE_4G_SETTING_SEND_COMMAND:
		if(setting_index >= nb_setting_command){
			// Reset Setting Command Idx
			setting_index = 0;
			return NETIF_OK;
		}
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
			last_time_sent = NETIF_GET_TIME_MS();
			// Clear Before Data
			netif_core_atcmd_reset(NETIF_4G, true);
			size = sprintf(at_message, setting_command[setting_index++]);
			utils_log_debug(at_message);
			netif_core_4g_output(at_message, size);
			setting_state = STATE_4G_SETTING_WAIT_FOR_RESPONSE;
		}
		break;
	case STATE_4G_SETTING_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			utils_log_error("Timeout\r\n");
			// Reset State
			setting_state = STATE_4G_SETTING_SEND_COMMAND;
			setting_retry = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
			if(response == NETIF_RESPONSE_OK){
				netif_core_atcmd_reset(NETIF_4G, true);
				setting_retry = 0;
				setting_state = STATE_4G_SETTING_SEND_COMMAND;
			}
			else if(response == NETIF_RESPONSE_ERROR){
				if(setting_retry >= NETIF_MAX_RETRY){
					netif_core_atcmd_reset(NETIF_4G, false);
					setting_retry = 0;
					setting_state = STATE_4G_SETTING_SEND_COMMAND;
					return NETIF_FAIL;
				}
				setting_retry ++;
				setting_state = STATE_4G_SETTING_SEND_COMMAND;
			}else {
				netif_core_atcmd_reset(NETIF_4G, false);
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

static netif_status_t netif_4g_imsi(){
	static uint32_t last_time_sent = 0;
	// Setting 4G module
	netif_core_response_t response;
	int size;
	switch (_4g_imsi_state) {
	case STATE_4G_IMSI_SEND_COMMAND:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
			last_time_sent = NETIF_GET_TIME_MS();
			// Clear Before Data
			netif_core_atcmd_reset(NETIF_4G, true);
			size = sprintf(at_message, NETIF_ATCMD_4G_GET_IMSI);
			utils_log_debug(at_message);
			netif_core_4g_output(at_message, size);
			_4g_imsi_state = STATE_4G_IMSI_WAIT_FOR_RESPONSE;
		}
		break;
	case STATE_4G_IMSI_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			utils_log_error("Timeout\r\n");
			// Reset State
			_4g_imsi_state = STATE_4G_IMSI_SEND_COMMAND;
			_4g_imsi_retry = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
			if(response == NETIF_RESPONSE_OK){
				if(netif_4g_parse_on_imsi(_4g_imsi, &_4g_imsi_length) == NETIF_OK){
					netif_core_atcmd_reset(NETIF_4G, true);
					_4g_imsi_retry = 0;
					_4g_imsi_completed = true;
					_4g_imsi_state = STATE_4G_IMSI_SEND_COMMAND;
					return NETIF_OK;
				}
			}
			else if(response == NETIF_RESPONSE_ERROR){
				if(_4g_imsi_retry >= NETIF_MAX_RETRY){
					netif_core_atcmd_reset(NETIF_4G, false);
					_4g_imsi_retry = 0;
					_4g_imsi_state = STATE_4G_IMSI_SEND_COMMAND;
					return NETIF_FAIL;
				}
				_4g_imsi_retry ++;
				_4g_imsi_state = STATE_4G_IMSI_SEND_COMMAND;
			}else {
				netif_core_atcmd_reset(NETIF_4G, false);
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

static netif_status_t netif_4g_iccid(){
	static uint32_t last_time_sent = 0;
	// Setting 4G module
	netif_core_response_t response;
	int size;
	switch (_4g_iccid_state) {
	case STATE_4G_ICCID_SEND_COMMAND:
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL){
			last_time_sent = NETIF_GET_TIME_MS();
			// Clear Before Data
			netif_core_atcmd_reset(NETIF_4G, true);
			size = sprintf(at_message, NETIF_ATCMD_4G_GET_ICCID);
			utils_log_debug(at_message);
			netif_core_4g_output(at_message, size);
			_4g_iccid_state = STATE_4G_ICCID_WAIT_FOR_RESPONSE;
		}
		break;
	case STATE_4G_ICCID_WAIT_FOR_RESPONSE:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			utils_log_error("Timeout\r\n");
			// Reset State
			_4g_iccid_state = STATE_4G_ICCID_SEND_COMMAND;
			_4g_iccid_retry = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Check Response
		if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
			if(response == NETIF_4G_REPORT_ICCID){
				if(netif_4g_parse_on_iccid(_4g_iccid, &_4g_iccid_length) == NETIF_OK){
					netif_core_atcmd_reset(NETIF_4G, true);
					_4g_iccid_retry = 0;
					_4g_iccid_completed = true;
					_4g_iccid_state = STATE_4G_ICCID_SEND_COMMAND;
					return NETIF_OK;
				}
			}
			else if(response == NETIF_RESPONSE_ERROR){
				if(_4g_iccid_retry >= NETIF_MAX_RETRY){
					netif_core_atcmd_reset(NETIF_4G, false);
					_4g_iccid_retry = 0;
					_4g_iccid_state = STATE_4G_ICCID_SEND_COMMAND;
					return NETIF_FAIL;
				}
				_4g_iccid_retry ++;
				_4g_iccid_state = STATE_4G_ICCID_SEND_COMMAND;
			}else {
				netif_core_atcmd_reset(NETIF_4G, false);
			}
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}

static netif_status_t netif_4g_idle(){
	netif_core_response_t response;
	// If Get Initialize response -> Restart 4G
	_4g_connected = true;
	if(netif_core_atcmd_is_responded(NETIF_4G, &response)){
		if(response == NETIF_4G_REPORT_INITIALIZE_DONE){
			netif_core_atcmd_reset(NETIF_4G, true);
			return NETIF_FAIL;
		}
	}
	return NETIF_OK;
}

static netif_status_t netif_4g_parse_on_imsi(uint8_t* imsi, uint32_t *imsiSize){
	uint8_t* onImsiBufferRef;
	uint32_t onImsiBufferLength = 0;

	char* outputBuffer[2];

	if(netif_core_atcmd_get_data_before(NETIF_4G, &onImsiBufferRef, &onImsiBufferLength))
	{
		if(utils_string_split_with_fixed_no(onImsiBufferRef, onImsiBufferLength, "\r\n",
											outputBuffer, 2, "OK"))
		{
			strcpy(imsi, outputBuffer[0]);
			*imsiSize = strlen(outputBuffer[0]);
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}

static netif_status_t netif_4g_parse_on_iccid(uint8_t* iccid, uint32_t *iccidSize){
	static uint8_t onIccidBuffer[NETIF_4G_ICCID_MAX_LENGTH + 32];
	static uint32_t onIccidBufferLength = 0;

	char* outputBuffer[2];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onIccidBuffer[onIccidBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onIccidBuffer, onIccidBufferLength, "\r\n",
											outputBuffer, 2, "OK"))
		{
			strcpy(iccid, outputBuffer[0]);
			*iccidSize = strlen(outputBuffer[0]);

			onIccidBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}

#endif
