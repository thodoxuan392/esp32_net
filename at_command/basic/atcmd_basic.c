/*
 * at_basic_command.c
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#include "stdio.h"
#include "at_basic_command.h"
#define AT_BASIC_BUFFER_LENGTH		128

static uint8_t at_basic_buffer[AT_BASIC_BUFFER_LENGTH];

char * ATCMD_basic_test_request(at_cmd_ops_t ops , at_cmd_req_basic_test_t test){
	(void)test;
	switch (ops) {
		case AT_CMD_OPS_SET:
			return NULL;
			break;
		case AT_CMD_OPS_EXEC:
			snprintf(at_basic_buffer, AT_BASIC_BUFFER_LENGTH,
					"AT\n"
					);
			break;
		case AT_CMD_OPS_QUERY:
			return NULL;
			break;
		default:
			break;
	}
	return at_basic_buffer;
}

char * ATCMD_basic_restart_request(at_cmd_ops_t ops, at_cmd_req_basic_restart_t restart){
	(void)restart;
	switch (ops) {
		case AT_CMD_OPS_SET:
			return NULL;
			break;
		case AT_CMD_OPS_EXEC:
			snprintf(at_basic_buffer, AT_BASIC_BUFFER_LENGTH,
					"AT+RST\n"
					);
			break;
		case AT_CMD_OPS_QUERY:
			return NULL;
			break;
		default:
			break;
	}
	return at_basic_buffer;
}


uint8_t ATCMD_basic_test_response(uint8_t * test_str,
										uint16_t test_str_len,
										at_cmd_rsp_basic_test_t *at_cmd_rsp_basic_test,
										at_cmd_ops_t *ops){


}
uint8_t ATCMD_basic_restart_response(uint8_t * restart_str ,
										uint16_t restart_str_len,
										at_cmd_rsp_basic_restart_t* at_cmd_rsp_basic_restart ,
										at_cmd_ops_t *ops){

}
