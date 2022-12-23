#ifndef NETIF_MQTT_H
#define NETIF_MQTT_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "netif_def.h"

#define NETIF_MQTT_PAYLOAD_LEN          2048
#define NETIF_MQTT_TOPIC_LEN            128

typedef struct{
    char * client_id;
    // Endpoint
    char * host;
    uint16_t port;
    uint16_t keep_alive;
    uint8_t clean_session;
    uint8_t reconnect;
    bool connected;
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
    void (*on_connect)(uint8_t result);
    void (*on_disconnect)(uint8_t result);
    void (*on_publish)(uint8_t result);
    void (*on_message)(char *topic, char* payload);
}netif_mqtt_client_t;



/**
 * @brief Inittialize MQTT Stack 
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_init();

/**
 * @brief Run MQTT Stack in supper loop, handle even, ...
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_run();

/**
 * @brief Deinitialize MQTT Stack
 * 
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_deinit();

// Specific Function
/**
 * @brief Config MQTT Client
 * 
 * @param mqtt_config Mqtt Client Config refer @netif_mqtt_client_t
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_config(netif_mqtt_client_t * client);

/**
 * @brief Client connect to Broker
 * 
 * @param client Pointer to mqtt client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_connect(netif_mqtt_client_t * client);

/**
 * @brief Disconnect from MQTT Broker
 * 
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_disconnect(netif_mqtt_client_t * client);

/**
 * @brief Subcribe topic
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param qos Qos of Subcribe
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_subcribe(netif_mqtt_client_t * client, char * topic , uint8_t qos);

/**
 * @brief Unsubcribe topic
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_unsubcribe(netif_mqtt_client_t * client, char *topic);

/**
 * @brief Publish Message to Topic 
 * 
 * @param client Pointer to mqtt_client
 * @param topic Topic name
 * @param payload Payload
 * @param qos Qos of Message
 * @param retain Retained Message
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_publish(netif_mqtt_client_t * client, char * topic , char * payload, uint8_t qos, uint8_t retain);

/**
 * @brief Check MQTT Client Connection
 * 
 * @param client Pointer to mqtt_client
 * @return netif_status_t Status of Process
 */
netif_status_t netif_mqtt_is_connected(netif_mqtt_client_t * client, bool *connected);

#endif
