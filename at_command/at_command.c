/*
 * at_command.c
 *
 *  Created on: Nov 29, 2022
 *      Author: xuanthodo
 */

#include "at_command.h"

char * at_command_parse_request(at_cmd_req_t request){
	char * result = NULL;
	switch (request.id) {
		case AT_CMD_ID_BASIC_TEST:
			result = ATCMD_basic_test_request(request.ops, request.data.at_cmd_req_basic_test);
			break;
		case AT_CMD_ID_RESTART:
			result = ATCMD_basic_restart_request(request.ops, request.data.at_cmd_req_basic_restart);
			break;
		default:
			break;
	}
	return result;
}

at_cmd_rsp_t at_command_parse_response(at_cmd_id_t at_cmd_id, uint8_t* response, uint16_t response_len){
	at_cmd_rsp_t at_cmd_rsp = {
			.id = AT_CMD_ID_UNKNOWN
	};
	switch (at_cmd_id) {
		case AT_CMD_ID_BASIC_TEST:
			if(ATCMD_basic_test_response(response,
											response_len,
											&at_cmd_rsp.data.at_cmd_rsp_basic_test,
											&at_cmd_rsp.ops)){
				at_cmd_rsp.id = AT_CMD_ID_BASIC_TEST;
			}
			break;
		case AT_CMD_ID_RESTART:
			if(ATCMD_basic_restart_response(response,
												response_len,
												&at_cmd_rsp.data.at_cmd_rsp_basic_restart,
												&at_cmd_rsp.ops)){
				at_cmd_rsp.id = AT_CMD_ID_RESTART;
			}
			break;
		default:
			break;
	}

	return at_cmd_rsp;
}
