#ifndef NETIF_4G_H
#define NETIF_4G_H

#include "stdbool.h"


// Generic Function
/**
 * @brief Initialize 4G 
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_4g_init();

/**
 * @brief Run 4G in Supper Loop
 * 
 * @return true If OK
 * @return false If failed
 */
bool netif_4g_run();

/**
 * @brief Deinitialize 4G
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_4g_deinit();


// Specific Function
/**
 * @brief Get IMEI of 4G Module
 * 
 * @param imei Pointer to Imei String Buffer
 * @param imei_max_size Max Size Imei String Buffer
 * @return true If OK
 * @return false If Failed or Timeout
 */
bool netif_4g_get_imei(char * imei, size_t imei_max_size);

/**
 * @brief Send Message to Other Phone
 * 
 * @param phone_number Phone Number of Target Sim 
 * @param message Message Sent
 * @return true If OK
 * @return false If Failed or Timeout
 */
bool netif_4g_send_sms(char * phone_number, char * message);

/**
 * @brief Get 4G Status Connection
 * 
 * @return true If 4G Connected
 * @return false If failed or timeout
 */
bool netif_4g_is_connected();

/**
 * @brief Get 4G Internet Status Connection
 * 
 * @return true If 4G internet is connected
 * @return false If failed or timeout
 */
bool netif_4g_is_internet_connected();



#endif //NETIF_4G_H