#include "manager/netif_wifi.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_wifi.h"
#include "netif_opts.h"
#include "utils/utils_buffer.h"
#include "utils/utils_logger.h"

// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
// Wifi Status
static bool wifi_connected = false;
static bool smartconfig_done = false;
static char smartconfig_ssid[SSID_LEN];
static char smartconfig_password[PASSWORD_LEN];
static char wifi_ip[IP_ADDR_LEN];
static char wifi_mac[MAC_ADDR_LEN];

/************************************ Internal Functions********************************/
// Setting Command
static char * setting_command[] = {
	NETIF_ATCMD_WIFI_RECONNECT_CONFIG
};
// Wifi Ethernet State
enum {

	STATE_WIFI_DISCONNECTED,
	STATE_WIFI_WAIT_FOR_CONNECTED,
	STATE_WIFI_CONNECTED,
};

static netif_status_t netif_wifi_fsm();
// Generic Function
/**
 * @brief Initialize Wifi Stack
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_init(){
	// TODO:
    return NETIF_OK;
}

/**
 * @brief Run Wifi Stack in Supper loop, handle event, ...
 * * Step:
 * 0: Enable Station Mode
 * 1: Wait Station Mode response
 * 2: Connect previous AP
 * 3: Wait Connect AP Response
 * 4: Start Smart Config
 * 5: Wait Start Smart Config Response
 * 6: Check Wifi Event
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_run(){
	// Handle Response
    netif_core_response_t at_response;
    uint8_t *data;
    size_t data_size;
    if(netif_core_atcmd_is_responded(&at_response)){
        switch (at_response)
        {
        case NETIF_WIFI_ETHERNET_REPORT_WIFI_CONNECTED:
        	utils_log_info("Wifi Connected\r\n");
            /* code */
            wifi_connected = true;
            netif_core_atcmd_reset(false);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_WIFI_GOT_IP:
        	utils_log_info("Wifi Got IP\r\n");
        	// Get data from Core Buffer
			netif_core_atcmd_get_data_before(&data,&data_size);
			// Get IP
			// TODO:
            netif_core_atcmd_reset(false);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_WIFI_DISCONNECTED:
        	utils_log_info("Wifi Disconnected\r\n");
            /* code */
            wifi_connected = false;
            netif_core_atcmd_reset(false);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_TYPE:
        	utils_log_info("Smartconfig Type\r\n");
            // Reset ATCMD
            netif_core_atcmd_reset(false);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_CONNECTED_AP:
        	utils_log_info("Smartconfig connected\r\n");
            // Set flag smartconfig_done to true
            smartconfig_done = true;
            // Reset ATCMD
            netif_core_atcmd_reset(false);
            break;
        case NETIF_WIFI_ETHERNET_REPORT_SMARTCONFIG_INFO:
        	utils_log_info("Smartconfig get info\r\n");
            smartconfig_done = true;
            // Get data from Core Buffer
            netif_core_atcmd_get_data_before(&data,&data_size);
            // Get SSID and Pass
            // TODO: Get SSID and Pass
            // Reset ATCMD
            netif_core_atcmd_reset(false);
            break;
        default:
            break;
        }
    }
    netif_wifi_fsm();
    return NETIF_IN_PROCESS;
}

/**
 * @brief Deinitialize Wifi Stack
 * @return true if OK
 * @return false if failed
 */
netif_status_t netif_wifi_deinit();




// Specific Function

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_station_mode(){
    static uint8_t step = 0;
    static uint32_t last_time_sent;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
    	last_time_sent = NETIF_GET_TIME_MS();
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_STATION_MODE);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT * 5){
			netif_core_atcmd_reset(true);
			utils_log_error("Timeout to receive station mode response");
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
        	// Reset State
        	step = 0;
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(true);
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
 * @brief Connect to SoftAP using SSID and Password
 * Step:
 * 0: Send request connect to Wifi Module
 * 1: Wait for response
 * @param ssid SSID of SoftAP
 * @param password Password of SoftAP
 * @param auth_mode Authentication Mode of SoftAP
 * @return true if connected
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_connect_ap(char *ssid, char * password){
    static uint8_t step = 0;
    static uint32_t last_time_sent;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
    	last_time_sent = NETIF_GET_TIME_MS();
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_CONNECT_AP, ssid, password);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Connect AP Response
        step = 1;
        break;
    case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
        // Wait Connect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Disconnect from AP
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_disconnect_ap(){
    static uint8_t step = 0;
    static uint32_t last_time_sent;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
    	last_time_sent = NETIF_GET_TIME_MS();
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_DISCONNECT_AP);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Check Connection Status of Wifi Status 
 * 
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_is_connected(bool * connected){
    if(wifi_connected){
        *connected = wifi_connected;
    }
    return NETIF_OK;
}

/**
 * @brief Check Connection Status of Wifi Status
 *
 * @return true If Station is connected to SoftAP
 * @return false If failed or timeout
 */
netif_status_t netif_wifi_reconnect(){
	static uint8_t step = 0;
	static uint32_t last_time_sent;
	netif_core_response_t at_response;
	int size;
	switch (step)
	{
	case 0:
		last_time_sent = NETIF_GET_TIME_MS();
		// Send Connect to AP to Wifi Module
		size = sprintf(at_message, NETIF_ATCMD_WIFI_RECONNECT);
		netif_core_wifi_ethernet_output(at_message, size);
		// Switch wait to Wait Disconnect AP Response
		step = 1;
		break;
	case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
		// Wait Reconnect AP Response
		if(netif_core_atcmd_is_responded(&at_response)){
			step = 0;
			// Donot use data from response -> Clean Core Buffer
			netif_core_atcmd_reset(false);
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
 * @brief Start SmartConfig 
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_start_smartconfig(){
    static uint8_t step = 0;
    static uint32_t last_time_sent;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
    	last_time_sent = NETIF_GET_TIME_MS();
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_START_SMARTCONFIG);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
        // Wait Disconnect AP Response
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

/**
 * @brief Stop Smartconfig
 * 
 * @return true  If OK
 * @return false if failed or timeout
 */
netif_status_t netif_wifi_stop_smartconfig(){
    static uint8_t step = 0;
    static uint32_t last_time_sent;
    netif_core_response_t at_response;
    int size;
    switch (step)
    {
    case 0:
    	last_time_sent = NETIF_GET_TIME_MS();
        // Send Connect to AP to Wifi Module
        size = sprintf(at_message, NETIF_ATCMD_WIFI_STOP_SMARTCONFIG);
        netif_core_wifi_ethernet_output(at_message, size);
        // Switch wait to Wait Disconnect AP Response
        step = 1;
        break;
    case 1:
		// Check Timeout
		if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT){
			// Reset State
			step = 0;
			// Return TIMEOUT
			return NETIF_TIMEOUT;

		}
        // Wait ATCMD Reponse
        if(netif_core_atcmd_is_responded(&at_response)){
            // Donot use data from response -> Clean Core Buffer
            netif_core_atcmd_reset(false);
            // Check AT Response
            if(at_response == NETIF_RESPONSE_OK){
                return NETIF_OK;
            }else{
                return NETIF_FAIL;
            }
        }
        step = 1;
        break;
    default:
        break;
    }
    return NETIF_IN_PROCESS;
}

static netif_status_t netif_wifi_fsm(){
	// Setting Wifi Module
	static size_t nb_setting_command = sizeof(setting_command) / sizeof(setting_command[0]);
	static uint8_t step = STATE_WIFI_WAIT_FOR_CONNECTED;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	static uint8_t current_command_idx = 0;
	netif_core_response_t response;
	netif_status_t ret;
	int size;
	switch (step) {
	case STATE_WIFI_DISCONNECTED:
		ret = netif_wifi_reconnect();
		if (ret != NETIF_IN_PROCESS){
			last_time_sent = NETIF_GET_TIME_MS();
			step = STATE_WIFI_WAIT_FOR_CONNECTED;
		}
		break;
	case STATE_WIFI_WAIT_FOR_CONNECTED:
		if(NETIF_GET_TIME_MS() - last_time_sent > 20000){
			last_time_sent = NETIF_GET_TIME_MS();
			utils_log_debug("Timeout\r\n");
			// Reset State
			step = STATE_WIFI_DISCONNECTED;
			break;
		}
		if(wifi_connected){
			step = STATE_WIFI_CONNECTED;
			break;
		}
		break;
	case STATE_WIFI_CONNECTED:
		if(!wifi_connected){
			step = STATE_WIFI_DISCONNECTED;
		}
		break;
	default:
		break;
	}
	return NETIF_IN_PROCESS;
}
