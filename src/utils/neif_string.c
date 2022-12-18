#include "utils/netif_string.h"

bool netif_string_is_receive_data(char *buffer , uint16_t buffer_len , const char * data){
	if(buffer == NULL || data == NULL){
		return false;
	}
	uint16_t data_len = strlen(data);
	uint16_t actual_buffer_len = strlen(buffer);
	// Validate valid buffer
	if(actual_buffer_len == 0 || actual_buffer_len < buffer_len || data_len == 0){
		return false;
	}
	else{
		for (uint16_t index = 0; index < data_len; index++) {
			if(buffer[buffer_len -data_len + index] != data[index]){
				return false;
			}
		}
	}
	return true;
}