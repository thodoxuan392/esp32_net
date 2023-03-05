#ifndef NETIF_ATCMD_MQTT_H
#define NETIF_ATCMD_MQTT_H


/***************************** Wifi-Ethernet MQTT AT Command***************************/

// Wifi-Ethernet Client Configuration (link_id,scheme,client_id,username,password)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_CLIENT_CONFIG        "AT+MQTTUSERCFG=0,%d,\"%s\",\"%s\",\"%s\",0,0,\"\"\n"
// Wifi-Ethernet Connect to MQTT Broker (link_id,host,port,reconnect)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_CONNECT_QUERY        "AT+MQTTCONN?\n"
// Wifi-Ethernet Connect to MQTT Broker (link_id,host,port,reconnect)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_CONNECT              "AT+MQTTCONN=0,\"%s\",%d,%d\n"
// Wifi-Ethernet Disconnect from MQTT Broker
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_DISCONNECT           "AT+MQTTCLEAN=0\n"
// Wifi-Ethernet Subcribe to Topic (link_id,topic,qos)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_SUBCRIBE             "AT+MQTTSUB=0,\"%s\",%d\n"
// Wifi-Ethernet Unsubcribe from Topic (link_id,topic)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_UNSUBCRIBE           "AT+MQTTUNSUB=0,\"%s\"\n"
// Wifi-Ethernet Publish Message to Topic (link_id,topic,data,qos,retain)
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_PUBLISH              "AT+MQTTPUB=0,\"%s\",\"%s\",%d,%d\n"
// Wifi-Ethernet Get MQTT Connection State
#define NETIF_ATCMD_WIFI_ETHERNET_MQTT_GET_CONNECTION       "AT+MQTTCONN?\n"


/***************************** 4G MQTT AT Command***************************/
// 4G Start MQTT Service
#define NETIF_ATCMD_4G_MQTT_START                           "AT+CMQTTSTART\r\n"
// 4G Stop MQTT Service
#define NETIF_ATCMD_4G_MQTT_STOP                            "AT+CMQTTSTOP\r\n"
// 4G Client Configuration (clientid)
#define NETIF_ATCMD_4G_MQTT_ACCQ                            "AT+CMQTTACCQ=0,\"%s\",0,3\r\n"
// 4G Connect to MQTT Broker (server_address, keepalive, clean_session, username, password)
#define NETIF_ATCMD_4G_MQTT_CONNECT                         "AT+CMQTTCONNECT=0,\"tcp://%s:%d\",%d,%d,\"%s\",\"%s\"\r\n"
// 4G Disconnect from MQTT Broker
#define NETIF_ATCMD_4G_MQTT_DISCONNECT                      "AT+CMQTTDISC=0,10000\r\n"  
// 4G Publish Message Topic (topic_len)
#define NETIF_ATCMD_4G_MQTT_PUBLISH_TOPIC                   "AT+CMQTTTOPIC=0,%d\r\n"    
// 4G Publish Message Payload (payload_len)
#define NETIF_ATCMD_4G_MQTT_PUBLISH_PAYLOAD                 "AT+CMQTTPAYLOAD=0,%d\r\n" 
// 4G Publish Message to MQTT Broker (qos, timeout, retain, dup)
#define NETIF_ATCMD_4G_MQTT_PUBLISH                         "AT+CMQTTPUB=0,%d,%d,%d,%d\r\n"
// 4G Subscribe Topic (topic_len,qos)
#define NETIF_ATCMD_4G_MQTT_SUBSCRIBE                 		"AT+CMQTTSUB=0,%d,%d\r\n"
// 4G Unsubscribe All Topic (topic_len)
#define NETIF_ATCMD_4G_MQTT_UNSUBSCRIBE_TOPIC                     "AT+CMQTTUNSUBOPIC=0,%d\r\n"


#endif
