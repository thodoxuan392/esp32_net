/*
 * at_command.h
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#ifndef ESP32_NET_AT_COMMAND_AT_COMMAND_H_
#define ESP32_NET_AT_COMMAND_AT_COMMAND_H_

#include "stdarg.h"
#include "at_command_def.h"
#include "basic/at_basic_command.h"
#include "wifi/at_wifi_command.h"
#include "ethernet/at_ethernet_command.h"
#include "tcpip/at_tcpip_command.h"

typedef struct {
	at_cmd_id_t id;
	at_cmd_ops_t ops;
	union {
		at_cmd_req_basic_test_t at_cmd_req_basic_test;
		at_cmd_req_basic_restart_t at_cmd_req_basic_restart;
	}data;
}at_cmd_req_t;

typedef struct {
	at_cmd_id_t id;
	at_cmd_ops_t ops;
	at_cmd_result_t result;
	union {
		at_cmd_rsp_basic_test_t at_cmd_rsp_basic_test;
		at_cmd_rsp_basic_restart_t at_cmd_rsp_basic_restart;
	}data;
}at_cmd_rsp_t;


char * at_command_parse_request(at_cmd_req_t request);
at_cmd_rsp_t at_command_parse_response(at_cmd_id_t at_cmd_id, uint8_t* response, uint16_t response_len);



#endif /* LIBS_ESP32_NET_AT_COMMAND_AT_COMMAND_H_ */
