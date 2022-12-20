#ifndef NETIF_H
#define NETIF_H

/**
 * @brief Network Interface for Ethernet, Wifi, 4G via UART
 * @author Xuan Tho Do
 * @date November 24, 2022
 */

#include "stdint.h"
#include "netif_opts.h"
#include "netif_def.h"
#include "apps/netif_http.h"
#include "apps/netif_mqtt.h"

/**
 * @brief Init Network Interface
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_init();

/**
 * @brief Run Network Interface in Super Loop
 * 
 * @return netif_status_t 
 */
netif_status_t netif_run();

/**
 * @brief Deinit Network Interface
 * 
 * @return netif_status_t 
 */
netif_status_t netif_deinit();

#endif // NETIF_H
