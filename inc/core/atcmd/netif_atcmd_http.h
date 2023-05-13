#ifndef NETIF_ATCMD_HTTP_H
#define NETIF_ATCMD_HTTP_H

/***************************** Wifi-Ethernet HTTP AT Command***************************/

// Wifi-Ethernet HTTP Get Request (url)
#define NETIF_ATCMD_WIFI_ETHERNET_HTTP_GET  "AT+HTTPCGET=%s\r\n"
// Wifi-Ethernet HTTP Post Request (url,length)
#define NETIF_ATCMD_WIFI_ETHERNET_HTTP_POST "AT+HTTPCPOST=%s,%d\r\n"
// Wifi-Ethernet HTTP Put Request (url,length)
#define NETIF_ATCMD_WIFI_ETHERNET_HTTP_PUT  "AT+HTTPCPUT=%s,%d\r\n"


/***************************** 4G HTTP AT Command***************************/
// 4G HTTP init session
#define NETIF_ATCMD_4G_HTTP_INIT            "AT+HTTPINIT\r\n"
// 4G HTTP terminate session
#define NETIF_ATCMD_4G_HTTP_TERMINATE       "AT+HTTPTERM\r\n"
// 4G HTTP Param (url)
#define NETIF_ATCMD_4G_HTTP_PARAM           "AT+HTTPPARA=\"URL\",%s\r\n"
// 4G HTTP Action (action)
#define NETIF_ATCMD_4G_HTTP_ACTION          "AT+HTTPACTION=%d\r\n"
// 4G HTTP Read Data (byte_size)
#define NETIF_ATCMD_4G_HTTP_READ            "AT+HTTPREAD=%d\r\n"
// 4G HTTP Write Data (byte_size)
#define NETIF_ATCMD_4G_HTTP_DATA            "AT+HTTPDATA=%d,10000\r\n"



#endif //NET_ATCMD_HTTP_H
