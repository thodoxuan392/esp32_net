#include "manager/netif_4g.h"




/**
 * @brief Initialize 4G 
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_init(){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Run 4G in Supper Loop
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_run(){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Deinitialize 4G
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_deinit(){
    //TODO: 
    return NETIF_OK;
}


// Specific Function
/**
 * @brief Get IMEI of 4G Module
 * 
 * @param imei Pointer to Imei String Buffer
 * @param imei_max_size Max Size Imei String Buffer
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_get_imei(char * imei, size_t imei_max_size){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Send Message to Other Phone
 * 
 * @param phone_number Phone Number of Target Sim 
 * @param message Message Sent
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_send_sms(char * phone_number, char * message){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Get 4G Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_connected(bool *connected){
    //TODO: 
    return NETIF_OK;
}

/**
 * @brief Get 4G Internet Status Connection
 * 
 * @return netif_status_t State of Process
 */
netif_status_t netif_4g_is_internet_connected(bool *connected){
    //TODO: 
    return NETIF_OK;
}