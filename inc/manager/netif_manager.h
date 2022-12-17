#ifndef NETIF_MANAGER_H
#define NETIF_MANAGER_H

// Generic Function
/**
 * @brief Initialize Net Manager
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_manager_init();

/**
 * @brief Run Net Manager in Supper Loop
 * 
 * @return true if OK
 * @return false if failed
 */
bool netif_manager_run();

/**
 * @brief Deinitialize Net Manager
 * 
 * @return true If OK
 * @return false If failed or timeout
 */
bool netif_manager_deinit();

#endif //NETIF_MANAGER_H