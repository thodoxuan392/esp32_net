#ifndef NETIF_BUFFER_H
#define NETIF_BUFFER_H

#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"

#define BUFFER_MAX_SIZE     2048

typedef struct {
    size_t tail;
    size_t head;
    uint8_t buffer[BUFFER_MAX_SIZE];
}netif_buffer_t;

/**
 * @brief Initialize Buffer
 * 
 * @param buffer Pointer to Buffer Object
 * @return true if OK
 * @return false if Failed
 */
bool netif_buffer_init(netif_buffer_t * buffer);

/**
 * @brief Push data to Buffer
 * 
 * @param buffer Pointer to Buffer
 * @param data Pointer to data in
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed
 */
bool netif_buffer_push(netif_buffer_t * buffer, uint8_t * data, size_t data_size);

/**
 * @brief Pop data from Buffer
 * 
 * @param buffer Pointer to Buffer
 * @param data Pointer to data out
 * @param data_size Data size
 * @return true If OK
 * @return false If failed
 */
bool netif_buffer_pop(netif_buffer_t  * buffer, uint8_t * data, size_t data_size);


/**
 * @brief Check Buffer is available or not
 * 
 * @param buffer Pointer to buffer
 * @return true if buffer available
 * @return false if buffer not available
 */
bool netif_buffer_is_available(netif_buffer_t * buffer);

#endif