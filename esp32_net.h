#ifndef ESP32_NET
#define ESP32_NET

/**
 * @brief Esp32 Network Interface for Ethernet and Wifi via UART
 * - Support MQTT Client
 * - Support HTTP Client
 * @author Xuan Tho Do
 * @date November 24, 2022
 */

#include "stdint.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/http_client.h"

/* Generic Definition*/
#define ESP32_NET_BUFFER_LEN                2048


/* Wifi Definition*/
#define ESP32_NET_WIFI_SSID_LEN             124
#define ESP32_NET_WIFI_PASSWD_LEN           124

typedef struct {
    uint8_t buffer[ESP32_NET_BUFFER_LEN];
    uint16_t buffer_len;
    esp32_net_config_t config;
}esp32_net_handle_t;


typedef struct{
    bool smartconfig_enable;                        // Smartconfig in wifi
    uint8_t ssid[ESP32_NET_WIFI_SSID_LEN];          // SSID
    uint16_t ssid_len;                              // SSID Length 
    uint8_t passwd[ESP32_NET_WIFI_PASSWD_LEN];      //  Password
    uint16_t passwd_len;                            // Password Len
}esp32_net_config_wifi_t;

typedef struct{
    
}esp32_net_config_ethernet_t;

typedef struct {
    esp32_net_config_wifi_t wifi_config;
    bool wifi_enable;
    esp32_net_config_ethernet_t ethernet_config;
    bool ethernet_enable;
}esp32_net_config_t;

/**
 * @brief Init ESP32 Network Stack 
 * 
 * @param handle Handler
 * @param config Configuration for esp32net
 */
void ESP32NET_init(esp32_net_handle_t *handle, esp32_net_config_t* config);

/**
 * @brief Put data to ESP32_Net to stack
 * 
 * @param handle Handler
 * @param data Pointer to input data
 * @param data_len Input data length
 */
void ESP32NET_input(esp32_net_handle_t *handle , uint8_t * data, size_t data_len);

/**
 * @brief Output Callback from Stack
 * 
 * @param handle Handler
 * @param data Pointer to output data
 * @param data_len Output datalen
 */
extern void ESP32NET_output(esp32_net_handle_t *handle, uint8_t *data, size_t data_len);


/**
 * @brief Loop process for data processing, it take 2-3ms in super loop
 * - Support Mqtt
 * - Support Http
 * @param handle Handler
 */
void ESP32NET_loop(esp32_net_handle_t *handle);

#endif