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
}netif_mqtt_config_t;



// Generic Function
bool netif_mqtt_init();
bool netif_mqtt_loop();

// Specific Function
bool netif_mqtt_config();
bool netif_mqtt_connect();
bool netif_mqtt_publish();
bool netif_mqtt_subcribe();
bool netif_mqtt_unsubcribe();
bool netif_mqtt_is_connected();
bool netif_mqtt_disconnect();

#endif