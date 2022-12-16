/*
 * at_wifi_command.h
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#ifndef LIBS_ESP32_NET_AT_COMMAND_WIFI_AT_WIFI_COMMAND_H_
#define LIBS_ESP32_NET_AT_COMMAND_WIFI_AT_WIFI_COMMAND_H_

#include "stdio.h"
#include "../at_command_def.h"


#define SSID_NAME_LENGTH			128
#define PASSWD_LENGTH				128
#define BSSID_LENGTH				32
#define MAC_LENGTH					32

/*
 * WIFI AT Command Data Structure
 */

// CWMODE
typedef struct {
	uint8_t mode;
	uint8_t auto_connect;
}at_cmd_req_wifi_cwmode_t;

typedef struct {
	uint8_t mode;
}at_cmd_rsp_wifi_cwmode_t;


// CWSTATE
typedef struct {
	// Nothing
}at_cmd_req_cwstate_t;

typedef struct {
	uint8_t state;
	uint8_t ssid[SSID_NAME_LENGTH];
	uint8_t ssid_length;
}at_cmd_rsp_cwstate_t;

// CWJAP
typedef struct {
	uint8_t ssid[SSID_NAME_LENGTH];
	uint8_t ssid_length;
	uint8_t passwd[PASSWD_LENGTH];
	uint8_t passwd_length;
	uint8_t bssid[PASSWD_LENGTH];
	uint8_t bssid_length;
	uint8_t rssi;
	uint8_t pci_en;
	uint16_t reconn_interval;
	uint8_t listen_interval;
	uint8_t scan_mode;
	uint8_t jap_timeout;
	uint8_t pmf;
}at_cmd_req_cwjap_t;

typedef struct {
	uint8_t ssid[SSID_NAME_LENGTH];
	uint8_t ssid_length;
	uint8_t passwd[PASSWD_LENGTH];
	uint8_t passwd_length;
	uint8_t bssid[PASSWD_LENGTH];
	uint8_t bssid_length;
	uint8_t rssi;
	uint8_t pci_en;
	uint16_t reconn_interval;
	uint8_t listen_interval;
	uint8_t scan_mode;
	uint8_t jap_timeout;
	uint8_t pmf;
	uint8_t error_code;
}at_cmd_rsp_cwjap_t;

// CWRECONNCFG
typedef struct {
	uint8_t interval_second;
	uint8_t repeat_count;
}at_cmd_req_cwreconncfg_t;

typedef struct {
	uint8_t interval_second;
	uint8_t repeat_count;
}at_cmd_rsp_cwreconncfg_t;

// CWLAP
typedef struct {
	uint8_t ssid[SSID_NAME_LENGTH];
	uint8_t ssid_length;
	uint8_t mac[MAC_LENGTH];
	uint8_t mac_length;
	uint8_t channel;
	uint8_t scan_type;
	uint16_t scan_time_min;
	uint16_t scan_time_max;
}at_cmd_req_cwlap_t;

typedef struct {
	uint8_t ecn;
	uint8_t ssid[SSID_NAME_LENGTH];
	uint8_t ssid_length;
	uint8_t rssi;
	uint8_t mac[MAC_LENGTH];
	uint8_t mac_length;
	uint8_t channel;
	uint8_t freq_offset;
	uint8_t feqcal_val;
	uint8_t pairwise_cipher;
	uint8_t group_cipher;
	uint8_t bgn;
	uint8_t wps;
}at_cmd_rsp_cwlap_t;


// CWQAP
typedef struct {
	// Nothing
}at_cmd_req_cwqap_t;

typedef struct {
	// Nothing
}at_cmd_rsp_cwqap_t;



#endif /* LIBS_ESP32_NET_AT_COMMAND_WIFI_AT_WIFI_COMMAND_H_ */
