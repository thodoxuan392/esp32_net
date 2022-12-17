#ifndef NETIF_H
#define NETIF_H

/**
 * @brief Network Interface for Ethernet, Wifi, 4G via UART
 * @author Xuan Tho Do
 * @date November 24, 2022
 */

#include "stdint.h"
#include "netif_opts.h"

bool netif_init();

bool netif_run();

bool netif_deinit();

bool netif_input(uint8_t * data, size_t data_len);

bool netif_output(uint8_t *data, size_t data_len);

#endif // NETIF_H
