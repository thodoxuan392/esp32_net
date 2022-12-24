#ifndef NETIF_ATCMD_MQTT_H
#define NETIF_ATCMD_MQTT_H

// Client Configuration (link_id,scheme,client_id,username,password)
#define NETIF_ATCMD_MQTT_CLIENT_CONFIG      "AT+MQTTUSERCFG=0,%d,%s,%s,%s,0,0,\"\"\n"
// Connect to MQTT Broker (link_id,host,port,reconnect)
#define NETIF_ATCMD_MQTT_CONNECT_QUERY    "AT+MQTTCONN?\n"
// Connect to MQTT Broker (link_id,host,port,reconnect)
#define NETIF_ATCMD_MQTT_CONNECT    "AT+MQTTCONN=0,%s,%d,%d\n"
// Disconnect from MQTT Broker
#define NETIF_ATCMD_MQTT_DISCONNECT "AT+MQTTCLEAN=0\n"
// Subcribe to Topic (link_id,topic,qos)
#define NETIF_ATCMD_MQTT_SUBCRIBE     "AT+MQTTSUB=0,%s,%d\n"
// Unsubcribe from Topic (link_id,topic)
#define NETIF_ATCMD_MQTT_UNSUBCRIBE      "AT+MQTTUNSUB=0,%s\n"
// Publish Message to Topic (link_id,topic,data,qos,retain)
#define NETIF_ATCMD_MQTT_PUBLISH        "AT+MQTTPUB=0,%s,%s,%d,%d\n"
// Get MQTT Connection State
#define NETIF_ATCMD_MQTT_GET_CONNECTION "AT+MQTTCONN?\n"

#endif
