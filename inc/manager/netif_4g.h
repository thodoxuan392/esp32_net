#ifndef NETIF_4G_H
#define NETIF_4G_H

#include "stdio.h"
#include "stdbool.h"
#include "netif_def.h"

#define NETIF_4G_IMEI_SIZE  15

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
 * @brief Deinitialize 4G
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_deinit();


// Specific Function
/**
 * @brief Get IMEI of 4G Module
 * 
 * @param imei Pointer to Imei String Buffer
 * @param imei_max_size Max Size Imei String Buffer
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_get_imei(char * imei, size_t imei_max_size);

/**
 * @brief Send Message to Other Phone
 * 
 * @param phone_number Phone Number of Target Sim 
 * @param message Message Sent
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_send_sms(char * phone_number, char * message);

/**
 * @brief Get 4G Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_connected(bool *connected);

/**
 * @brief Get 4G Internet Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_internet_connected(bool *connected);



#endif //NETIF_4G_H