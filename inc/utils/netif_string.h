#ifndef NETIF_STRING_H
#define NETIF_STRING_H

#include "stdint.h"
#include "stdbool.h"
#include "string.h"


/**
 * @brief Compare Buffer with Len and Data String from the end of buffer to start of Buffer
 * 
 * @param buffer Pointer to buffer 
 * @param buffer_len Buffer Len
 * @param data String
 * @return true if Matched
 * @return false if not Matched
 */
bool netif_string_is_receive_data(char *buffer , uint16_t buffer_len , const char * data);

#endif //NETIF_STRING_H