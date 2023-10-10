#ifndef NETIF_4G_H
#define NETIF_4G_H

#include "stdio.h"
#include "stdbool.h"
#include "netif_def.h"
#include "netif_opts.h"

#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1


/**
 * @brief Initialize 4G 
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_init();

/**
 * @brief Run 4G in Supper Loop
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_run();

/**
 * @brief Reset 4G module
 *
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_reset();

/**
 * @brief Deinitialize 4G
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_deinit();


/**
 * @brief Get 4G Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_connected(bool *connected);

#endif

#endif //NETIF_4G_H
