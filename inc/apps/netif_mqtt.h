#ifndef NETIF_MQTT_H
#define NETIF_MQTT_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

#define NETIF_MQTT_PAYLOAD_LEN          2048
#define NETIF_MQTT_TOPIC_LEN            128

typedef struct{
    uint8_t client_id;
    // Endpoint
    char * host;
    uint16_t port;
    uint16_t keep_alive;
    uint8_t clean_session;
    uint8_t reconnect;
    // Lasswill Message
    char *lwt_topic;
    char *lwt_msg;
    uint8_t lwt_qos;
    uint8_t lwt_retain;
    // Authentication
    char *username;
    char *password;
    // Secure Connection is disable
    char * ca;
    char * client_cert;
    char * client_key;
    // Callback function
    void (*on_publish)(uint8_t result);
    void (*on_subcribe)(uint8_t result);
    void (*on_connect)(uint8_t result);
    void (*on_disconnect)(uint8_t result);
}netif_mqtt_client_t;



/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return true if Ok
 * @return false if failed or timeout
 */
bool netif_mqtt_init();

/**
 * @brief Run MQTT Stack in supper loop, handle even, ...
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_mqtt_run();

/**
 * @brief Deinitialize MQTT Stack
 * 
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_mqtt_deinit();

// Specific Function
/**
 * @brief Config MQTT Client
 * 
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return true if OK
 * @return false if failed or timeout
 */
bool netif_mqtt_config(netif_mqtt_client_t * client);
bool netif_mqtt_connect(netif_mqtt_client_t * client);
bool netif_mqtt_disconnect(netif_mqtt_client_t * client);
bool netif_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos);
bool netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic);
bool netif_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain);
bool netif_mqtt_is_connected(netif_mqtt_client_t * client);

#endif