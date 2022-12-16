/*
 * at_command_def.h
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#ifndef LIBS_ESP32_NET_AT_COMMAND_AT_COMMAND_DEF_H_
#define LIBS_ESP32_NET_AT_COMMAND_AT_COMMAND_DEF_H_

/*
 * AT Command Identifier
 */
typedef enum {
	// Basic
	AT_CMD_ID_BASIC_TEST = 0,				// Command Test
	AT_CMD_ID_RESTART,						// Restart Module
	// Wifi
	AT_CMD_ID_WIFI_CWMODE ,					// Query/Set the Wifi Mode (Station/SoftAP/Station + SoftAP)
	AT_CMD_ID_WIFI_CWSTATE,					// Query the Wifi State and Wifi information
	AT_CMD_ID_WIFI_CWJAP,					// Connect to an AP
	AT_CMD_ID_WIFI_CWRECONNCFG, 			// Query/Set the Wifi Reconnecting Configuration
	AT_CMD_ID_WIFI_CWLAP,					// List Availables APs
	AT_CMD_ID_WIFI_CWQAP, 					// Disconnect from AP
	AT_CMD_ID_WIFI_START_SMARTCONFIG,		// Start Smart Config
	AT_CMD_ID_WIFI_STOP_SMARTCONFIG,		// Stop Smart Config

	// Ethernet
	AT_CMD_ID_ETHERNET_CIPETHMAC,			// Query/Set the MAC Address of the ESP32 Ethernet
	AT_CMD_ID_ETHERNET_CIPETH,				// Query/Set the IP Address of ESP32 Ethernet,

	// TCP/IP
	AT_CMD_ID_TCPIP_CIPV6,					// Enable/Disable IPv6
	AT_CMD_ID_TCPIP_CIPSTATE,				// Obtain TCP/UDP/SSL Connection Information
	AT_CMD_ID_TCPIP_CIPSTATUS,				// Obtain TCP/UDP/SSL Connection Status
	AT_CMD_ID_TCPIP_CIPDOMAIN,				// Resolve Domain Name
	AT_CMD_ID_TCPIP_CIPSTART,				// Start a TCP/UDP/SSL Connection
	AT_CMD_ID_TCPIP_CIPSTARTEX,				// Start a TCP/UDP/SSL Connection with automatically ID
	AT_CMD_ID_TCPIP_CIPSEND,				// Send data in Normal Transmission
	AT_CMD_ID_TCPIP_CIPCLOSE,				// Close TCP/UDP/SSL Connection

	// UNKNOWN
	AT_CMD_ID_UNKNOWN						// UNKNOWN
}at_cmd_id_t;

/*
 * AT Command Operator
 */
typedef enum {
	// Wifi
	AT_CMD_OPS_SET = 0,
	AT_CMD_OPS_EXEC,
	AT_CMD_OPS_QUERY
}at_cmd_ops_t;

/*
 * AT Command Result
 */
typedef enum {
	// Wifi
	AT_CMD_RESULT_OK = 0,
	AT_CMD_RESULT_ERROR,
	AT_CMD_RESULT_TIMEOUT
}at_cmd_result_t;



#endif /* LIBS_ESP32_NET_AT_COMMAND_AT_COMMAND_DEF_H_ */
