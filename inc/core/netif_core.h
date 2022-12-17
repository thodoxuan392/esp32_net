#ifndef NETIF_CORE_H
#define NETIF_CORE_H


#include "netif_opts.h"

// Generic Function
/**
 * @brief Initialize Network Interface Core Stack
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_core_init();

/**
 * @brief Run Network Interface Core Stack in Supper Loop
 * 
 * @return true If OK
 * @return false If failed
 */
bool netif_core_run();

/**
 * @brief Deinitialize Network Interface Core Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_core_deinit();


// Specific Function
/**
 * @brief 
 * 
 * @param data 
 * @param data_size 
 * @return true 
 * @return false 
 */
bool netif_core_4g_input(uint8_t* data, size_t data_size);
bool netif_core_4g_output(uint8_t* data, size_t data_size);
bool netif_core_wifi_ethernet_input(uint8_t* data, size_t data_size);
bool netif_core_wifi_ethernet_output(uint8_t * data, size_t data_size);

#endif