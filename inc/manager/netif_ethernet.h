#ifndef NETIF_ETHERNET_H
#define NETIF_ETHERNET_H

#include "stdbool.h"

/**
 * @brief Initialize Ethernet Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_ethernet_init();

/**
 * @brief Run Ethernet Stack in Supper Loop, handle event,...
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_ethernet_run();

/**
 * @brief Deinitialize Ethernet Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_ethernet_deinit();

/**
 * @brief Get Ethernet IP Address 
 * 
 * @param ip Pointer to IP String Buffer 
 * @param ip_max_size Max Size of IP String Buffer
 * @return true If Get IP OKe
 * @return false If Get Ip failed or Timeout
 */
bool netif_ethernet_get_ip(char * ip, size_t ip_max_size);


/**
 * @brief Get Ethernet Mac Address
 * 
 * @param mac Pointer to MAC Address
 * @param mac_max_size Mac Size of Mac String Buffer
 * @return true 
 * @return false 
 */
bool netif_ethernet_get_mac(char * mac, size_t mac_max_size);

#endif //NETIF_ETHERNET_H