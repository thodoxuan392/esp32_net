#include "manager/netif_manager.h"
#include "manager/netif_4g.h"
#include "manager/netif_ethernet.h"
#include "manager/netif_wifi.h"
#include "netif_opts.h"
#include "utils/utils_logger.h"

static netif_manager_mode_t netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;

// Internal Network Manager State
enum {
	NETIF_MANAGER_INIT_4G,
	NETIF_MANAGER_INIT_ETHERNET,
	NETIF_MANAGER_INIT_WIFI,
	NETIF_MANAGER_CHECK_4G_CONNECTION,
	NETIF_MANAGER_CHECK_ETHERNET_CONNECTION,
	NETIF_MANAGER_CHECK_WIFI_CONNECTION,
	NETIF_MANAGER_WIFI_RUN_STATION_MODE,
	NETIF_MANAGER_WIFI_START_SMARTCONFIG
};
// Internal Function
static void netif_manager_disconnect_mode();
static void netif_manager_4g_mode();
static void netif_manager_ethernet_mode();
static void netif_manager_wifi_mode();


/**
 * @brief Initialize Net Manager
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_init(){
    if(netif_4g_init() != NETIF_OK){
        utils_log_error("netif_4g_init failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_ethernet_init() != NETIF_OK){
        utils_log_error("netif_ethernet_init failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_wifi_init() != NETIF_OK){
        utils_log_error("netif_wifi_init failed\r\n");
        return NETIF_FAIL;
    }
    return NETIF_OK;
}

/**
 * @brief Run Net Manager in Supper Loop
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_run(){
    // Run All Stack for handling Event
    netif_4g_run();
    netif_wifi_run();
    netif_ethernet_run();
    switch (netif_manager_mode)
    {
    case NETIF_MANAGER_DISCONNECTED_MODE:
        netif_manager_disconnect_mode();
        break;
    case NETIF_MANAGER_4G_MODE:
        netif_manager_4g_mode();
        break;
    case NETIF_MANAGER_ETHERNET_MODE:
        netif_manager_ethernet_mode();
    case NETIF_MANAGER_WIFI_MODE:
        netif_manager_wifi_mode();
        break;
    default:
        break;
    }
}

/**
 * @brief Deinitialize Net Manager
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_deinit(){
    // Do something
    return NETIF_OK;
}


/**
 * @brief Check if is running 4G Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_connect_to_internet(bool *connected){
    if(netif_manager_mode != NETIF_MANAGER_DISCONNECTED_MODE){
        *connected = true;
    }else{
        *connected = false;
    }
    return NETIF_OK;
}

/**
 * @brief Check if is running 4G Mode
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_4g_mode(bool *connected){
    if(netif_manager_mode == NETIF_MANAGER_4G_MODE){
        *connected = true;
    }else{
        *connected = false;
    }
    return NETIF_OK;
}


/**
 * @brief Check if is running Wifi Mode
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_ethernet_mode(bool *connected){
    if(netif_manager_mode == NETIF_MANAGER_ETHERNET_MODE){
        *connected = true;
    }else{
        *connected = false;
    }
    return NETIF_OK;
}

/**
 * @brief Check if is running Wifi Mode
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_manager_is_wifi_mode(bool *connected){
    if(netif_manager_mode == NETIF_MANAGER_WIFI_MODE){
        *connected = true;
    }else{
        *connected = false;
    }
    return NETIF_OK;
}


/**
 * @brief Get Current State (Mode) of Network Manager
 *
 * @return netif_manager_mode_t network mode
 */
netif_manager_mode_t netif_manager_get_mode(){
    return netif_manager_mode;
}




// Internal Function
/**
 * @brief Disconnect Mode Operation every 10 seconds
 * step
 * 0: Check 4G Connection
 * 0.1: If 4G have Connection -> Switch to Run 4G Mode
 * 0.2: If 4G dont have Connection -> Step 1
 * 1: Check Ethernet Connection
 * 1.1: If Ethernet have Connection -> Switch to Ethernet-Wifi Mode
 * 1.2: If Ethernet don't have Connection -> Step 2
 * 2: Check Wifi Connection 
 * 2.1: If Wifi have connection -> Switch to Ethernet-Wifi Mode
 * 2.2: If Wifi don't have a connection -> Step 3
 * 3. Run Wifi as Station Mode -> Step 4
 * 4: Start Smartconfig -> Step 0
 * 
 */
static void netif_manager_disconnect_mode(){
    static uint32_t previous_time = 0;
    static uint8_t step = 0;
    static bool smartconfig_started = false;
    netif_status_t ret;
    bool _4g_connected = false;
    bool _wifi_connected = false;
    bool _ethernet_connected = false;
    // If Previous time to Current time is less than 10 seconds -> Return
    if(NETIF_GET_TIME_MS() - previous_time < NETIF_MANAGER_RETRY_INTERVAL){
        return;
    }
    // Assign Previous time = Current Time
    switch (step)
    {
    case 0:
        //Check 4G Connection
        ret = netif_4g_is_connected(&_4g_connected);
        if(ret == NETIF_OK){
        	previous_time = NETIF_GET_TIME_MS();
            // If 4G have Connection -> Switch to Run 4G Mode
            if(_4g_connected){
            	utils_log_info("4G connected\r\n");
                netif_manager_mode = NETIF_MANAGER_4G_MODE;
                return NETIF_OK;
            }
            // If 4G dont have Connection -> Step 1
            else{
            	utils_log_debug("4G not connected\r\n");
                step = 1;
            }
        }else if(ret != NETIF_IN_PROCESS){
        	previous_time = NETIF_GET_TIME_MS();
        	utils_log_error("Check 4G Connection failed\r\n");
        }
        break;
    case 1:
        // Check Ethernet Connection
        ret = netif_ethernet_is_connected(&_ethernet_connected);
        if(ret == NETIF_OK){
        	previous_time = NETIF_GET_TIME_MS();
            //If Ethernet have Connection -> Switch to Ethernet-Wifi Mode
            if(_ethernet_connected){
            	utils_log_info("Ethernet connected\r\n");
                netif_manager_mode = NETIF_MANAGER_ETHERNET_MODE;
                return NETIF_OK;
            }
            // If Ethernet don't have Connection -> Step 2
            else{
            	utils_log_debug("Ethernet not connected\r\n");
                step = 2;
            }
        }else if(ret != NETIF_IN_PROCESS){
        	previous_time = NETIF_GET_TIME_MS();
        	utils_log_error("Check Ethernet Connection Failed\r\n");
        	step = 2;
        }
        break;
    case 2:
        // Check Wifi Connection
        ret = netif_wifi_is_connected(&_wifi_connected);
        if(ret == NETIF_OK){
        	previous_time = NETIF_GET_TIME_MS();
            if(_wifi_connected){
            	utils_log_info("Wifi connected\r\n");
                netif_manager_mode = NETIF_MANAGER_WIFI_MODE;
                return NETIF_OK;
            }else{
            	utils_log_error("Wifi not connected\r\n");
            	// Check Start Config Run Before
				if(!smartconfig_started){
					step = 3;	// Switch to SmartConfig
				}else{
					step = 0;	// Reset to check 4g connection
				}
            }
        }else if(ret != NETIF_IN_PROCESS){
        	previous_time = NETIF_GET_TIME_MS();
        	utils_log_error("Check Wifi Connection Failed\r\n");
        	step = 0;
        }
        break;
    case 3:
        // Run Wifi as Station Mode
        ret = netif_wifi_station_mode();
        if(ret == NETIF_OK){
        	previous_time = NETIF_GET_TIME_MS();
        	utils_log_info("Run Wifi as Station Mode OK\r\n");
        	step = 4;
        }else if(ret != NETIF_IN_PROCESS){
        	previous_time = NETIF_GET_TIME_MS();
        	utils_log_error("Run Wifi as Station Mode Failed\r\n");
        	step = 0;
        }
        break;
    case 4:
		// Start Smartconfig
		ret = netif_wifi_start_smartconfig();
		if(ret != NETIF_IN_PROCESS){
			smartconfig_started = true;
			previous_time = NETIF_GET_TIME_MS();
			utils_log_info("Started Smartconfig OK\r\n");
			step = 0;
		}
        break;
    default:
        break;
    }

}

/**
 * @brief Run in 4G Mode, check connection every 10 seconds
 * Step:
 * 0: Check 4G Connection
 * 0.1: If 4G Connection Lost -> Swtich to Disconnected Mode
 * 0.2: If 4G Connection Keeped -> Nothing to do
 */
static void netif_manager_4g_mode(){
    static uint32_t previous_time = 0;
    netif_status_t ret;
    bool _4g_connected = false;
    bool _wifi_connected = false;
    bool _ethernet_connected = false;
    // If Previous time to Current time is less than 10 seconds -> Return
    if(NETIF_GET_TIME_MS() - previous_time < NETIF_MANAGER_RETRY_INTERVAL){
        return;
    }
    // Assign Previous time = Current Time
    previous_time = NETIF_GET_TIME_MS();
    // Check 4G Connection
    ret = netif_4g_is_connected(&_4g_connected);
    if(ret == NETIF_OK){
        // If 4G have Connection -> Do nothing
        if(_4g_connected){
            return;
        }
        // If 4G dont have Connection -> Switch to Disconnected Mode
        else{
            netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;
        }
    }
}



/**
 * @brief Run in Ethernet Mode, check connection every 10 seconds
 * Step:
 * 1: Check Ethernet Connection
 * 1.1: If Ethernet Connection Keeped -> Do nothing
 * 1.2: If Ethernet Connection Lost -> Switch to Disconnected Mode
 */
static void netif_manager_ethernet_mode(){
    static uint32_t previous_time = 0;
    static uint8_t step = 0;
    netif_status_t ret;
    bool _4g_connected = false;
    bool _wifi_connected = false;
    bool _ethernet_connected = false;
    // If Previous time to Current time is less than 10 seconds -> Return
    if(NETIF_GET_TIME_MS() - previous_time < NETIF_MANAGER_RETRY_INTERVAL){
        return;
    }
    // Assign Previous time = Current Time
    ret = netif_ethernet_is_connected(&_ethernet_connected);
	if(ret == NETIF_OK){
		//If Ethernet have Connection -> Switch to Disconnected Mode
		if(!_ethernet_connected){
			netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;
			utils_log_debug("Ethernet not connected\r\n");
		}
		previous_time = NETIF_GET_TIME_MS();
	}else if(ret != NETIF_IN_PROCESS){
		utils_log_error("Check Ethernet Connection Failed\r\n");
		netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;
		previous_time = NETIF_GET_TIME_MS();
	}
}

/**
 * @brief Run in Wifi Mode, check connection every 10 seconds
 * Step:
 * 0: Check 4G Connection
 * 0.1: If 4G Connected -> Switch to 4G Mode
 * 0.2: If 4G Connection Lost -> Step 1
 * 1: Check Ethernet Connection
 * 1.1: If Ethernet Connection Keeped -> Do nothing
 * 1.2: If Ethernet Connection Lost -> Step 2
 * 2: Check Wifi Connection
 * 2.1: If Ethernet Connection Keeped -> Do nothing
 * 2.2: If Ethernet Connection Lost -> Switch to Disconnected to Check from Begin
 */
static void netif_manager_wifi_mode(){
    static uint32_t previous_time = 0;
    static uint8_t step = 0;
    netif_status_t ret;
    bool _4g_connected = false;
    bool _wifi_connected = false;
    bool _ethernet_connected = false;
    // If Previous time to Current time is less than 10 seconds -> Return
    if(NETIF_GET_TIME_MS() - previous_time < NETIF_MANAGER_RETRY_INTERVAL){
        return;
    }
    // Assign Previous time = Current Time
    ret = netif_wifi_is_connected(&_wifi_connected);
	if(ret == NETIF_OK){
		if(!_wifi_connected){
			utils_log_debug("Wifi not connected\r\n");
			// Switch to Disconnected Mode
			netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;
		}
		previous_time = NETIF_GET_TIME_MS();
	}else if(ret != NETIF_IN_PROCESS){
		utils_log_error("Check Wifi Connection Failed\r\n");
		netif_manager_mode = NETIF_MANAGER_DISCONNECTED_MODE;
		previous_time = NETIF_GET_TIME_MS();
	}
}


