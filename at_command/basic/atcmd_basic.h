/*
 * at_basic_command.h
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#ifndef LIBS_ESP32_NET_AT_COMMAND_BASIC_AT_BASIC_COMMAND_H_
#define LIBS_ESP32_NET_AT_COMMAND_BASIC_AT_BASIC_COMMAND_H_

#include "stdio.h"
#include "at_basic_command.h"
#include "../at_command_def.h"

/*
 * Basic AT Command Data Structure
 */
typedef struct {
	// Nothing
}at_cmd_req_basic_test_t;

typedef struct {
	// Nothing
}at_cmd_rsp_basic_test_t;

typedef struct {
	// Nothing
}at_cmd_req_basic_restart_t;

typedef struct {
	// Nothing
}at_cmd_rsp_basic_restart_t;

char * ATCMD_basic_test_request(at_cmd_ops_t ops, at_cmd_req_basic_test_t test);
char * ATCMD_basic_restart_request(at_cmd_ops_t ops, at_cmd_req_basic_restart_t restart);

uint8_t ATCMD_basic_test_response(uint8_t * test_str , uint16_t test_str_len, at_cmd_rsp_basic_test_t *at_cmd_rsp_basic_test ,at_cmd_ops_t *ops);
uint8_t ATCMD_basic_restart_response(uint8_t * restart_str , uint16_t restart_str_len, at_cmd_rsp_basic_restart_t* at_cmd_rsp_basic_restart , at_cmd_ops_t *ops);



#endif /* LIBS_ESP32_NET_AT_COMMAND_BASIC_AT_BASIC_COMMAND_H_ */
