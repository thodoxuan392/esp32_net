/*
 * at_parse_common.c
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */


#include "at_parse_common.h"

typedef enum {
	OK,
	ERROR,
	MAX_IDX
};

static char * at_response_str[MAX_IDX] = {
		"OK",
		"ERROR"
};

bool ATPARSE_is_ok(uint8_t *data , uint16_t data_len, uint8_t ** data_out){


	*data_out = strnstr(data, at_response_str[OK], data_len);
	if(*data_out){
		return true;
	}
	return false;
}


bool ATPARSE_is_error(uint8_t *data , uint16_t data_len, uint8_t ** data_out){
	*data_out = strnstr(data, at_response_str[ERROR], data_len);
	if(*data_out){
		return true;
	}
	return false;
}
