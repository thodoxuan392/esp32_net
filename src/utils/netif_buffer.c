#include "utils/netif_buffer.h"
#include "string.h"



/**
 * @brief Initialize Buffer
 * 
 * @param buffer Pointer to Buffer Object
 * @return true if OK
 * @return false if Failed
 */
bool netif_buffer_init(netif_buffer_t * buffer){
    memset(buffer , 0, sizeof(netif_buffer_t));
}

/**
 * @brief Push data to Buffer
 * 
 * @param buffer Pointer to Buffer
 * @param data Pointer to data in
 * @param data_size Data Size
 * @return true if OK
 * @return false if Failed
 */
bool netif_buffer_push(netif_buffer_t * buffer, uint8_t * data, size_t data_size){
    for (size_t i = 0; i < data_size; i++)
    {
        buffer->buffer[buffer->head] = data[i];
        buffer->head = (buffer->head + 1) % BUFFER_MAX_SIZE;
    }
    return true;
}

/**
 * @brief Pop data from Buffer
 * 
 * @param buffer Pointer to Buffer
 * @param data Pointer to data out
 * @param data_size Data size
 * @return true If OK
 * @return false If failed
 */
bool netif_buffer_pop(netif_buffer_t  * buffer, uint8_t * data, size_t data_size){
    for (size_t i = 0; i < data_size; i++)
    {
        /* code */
        data[i] = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_MAX_SIZE;
    }
    return true;
}


/**
 * @brief Check Buffer is available or not
 * 
 * @param buffer Pointer to buffer
 * @return true if buffer available
 * @return false if buffer not available
 */
bool netif_buffer_is_available(netif_buffer_t * buffer){
    if(buffer->head != buffer->tail){
        return true;
    }
    return false;
}
