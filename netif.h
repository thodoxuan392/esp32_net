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
#include "app_debug.h"


/* Generic Definition*/
#define ESP32_NET_AT_BUFFER_LEN                2048
#define ESP32_NET_AT_CMD_RSP_LEN               20

typedef enum {
	ESP32NET_DISCONNECTED = 0,
	ESP32NET_ETHERNET_CONNECTED,
	ESP32NET_WIFI_CONNECTED,
	ESP32NET_WIFI_WAIT_SMARTCONFIG,
}esp32_state_t;

/**
 * @brief Init ESP32 Network Stack 
 * 
 * @param handle Handler
 * @param config Configuration for esp32net
 */
void esp32_netmanager_init();

/**
 * @brief Put data to ESP32_Net to stack
 * 
 * @param handle Handler
 * @param data Pointer to input data
 * @param data_len Input data length
 */
void esp32_netmanager_input(uint8_t * data, size_t data_len);

/**
 * @brief Output Callback from Stack
 * 
 * @param handle Handler
 * @param data Pointer to output data
 * @param data_len Output datalen
 */
 void esp32_netmanager_output(uint8_t *data, size_t data_len);


/**
 * @brief Loop process for data processing, it take 2-3ms in super loop
 * - Support Mqtt
 * - Support Http
 * @param handle Handler
 */
void esp32_netmanager_loop();

#endif
