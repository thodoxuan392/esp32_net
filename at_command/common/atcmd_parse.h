/*
 * at_parse_common.h
 *
 *  Created on: Dec 1, 2022
 *      Author: xuanthodo
 */


#ifndef AT_PARSE_COMMON_H
#define AT_PARSE_COMMON_H
#include "stdbool.h"
#include "stdio.h"

bool ATPARSE_is_ok(uint8_t *data , uint16_t data_len, uint8_t ** data_out);
bool ATPARSE_is_error(uint8_t *data , uint16_t data_len, uint8_t ** data_out);

#endif #AT_PARSE_COMMON_H
