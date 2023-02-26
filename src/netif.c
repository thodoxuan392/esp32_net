#include "netif.h"
#include "apps/netif_http.h"
#include "apps/netif_mqtt.h"
#include "manager/netif_manager.h"
#include "core/netif_core.h"
#include "utils/utils_logger.h"

/**
 * @brief Init Network Interface
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_init(){
    if(netif_mqtt_init() != NETIF_OK){
        utils_log_error("netif_mqtt_init failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_http_init() != NETIF_OK){
        utils_log_error("netif_http_init failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_manager_init() != NETIF_OK){
        utils_log_error("netif_manager_init failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_core_init() != NETIF_OK){
        utils_log_error("netif_core_init failed\r\n");
        return NETIF_FAIL;
    }
    return NETIF_OK;
}

/**
 * @brief Run Network Interface in Super Loop
 * 
 * @return netif_status_t 
 */
netif_status_t netif_run(){
    netif_http_run();
    netif_mqtt_run();
    netif_manager_run();
    netif_core_run();
}

/**
 * @brief Deinit Network Interface
 * 
 * @return netif_status_t 
 */
netif_status_t netif_deinit(){
    
    if(netif_mqtt_deinit() != NETIF_OK){
        utils_log_error("netif_mqtt_deinit failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_http_deinit() != NETIF_OK){
        utils_log_error("netif_http_deinit failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_manager_deinit() != NETIF_OK){
        utils_log_error("netif_manager_deinit failed\r\n");
        return NETIF_FAIL;
    }
    if(netif_core_deinit() != NETIF_OK){
        utils_log_error("netif_core_deinit failed\r\n");
        return NETIF_FAIL;
    }
    return NETIF_OK;
}