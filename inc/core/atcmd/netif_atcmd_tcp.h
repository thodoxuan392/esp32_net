#ifndef NETIF_ATCMD_TCP_H
#define NETIF_ATCMD_TCP_H

/***************************** 4G TCP AT Command***************************/
#define NETIF_ATCMD_4G_TCP_START_SOCKET_SERVICE         "AT+NETOPEN\r\n"
#define NETIF_ATCMD_4G_TCP_STOP_SOCKET_SERVICE          "AT+NETCLOSE\r\n"
#define NETIF_ATCMD_4G_TCP_CONNECT                      "AT+CIPOPEN=%d,\"TCP\",\"%s\",%d,%d\r\n"
#define NETIF_ATCMD_4G_TCP_DISCONNECT                   "AT+CIPCLOSE=%d\r\n"
#define NETIF_ATCMD_4G_TCP_SEND                         "AT+CIPSEND=%d,%d\r\n"
#define NETIF_ATCMD_4G_TCP_SETUP_RX_DATA_MODE           "AT+CIPRXGET=1\r\n"
#define NETIF_ATCMD_4G_TCP_READ_DATA_WITH_LENGTH        "AT+CIPRXGET=2,%d,%d\r\n"
#define NETIF_ATCMD_4G_TCP_READ_DATA_WITHOUT_LENGTH     "AT+CIPRXGET=2,%d\r\n"


/***************************** 4G TCP AT Command***************************/
#define NETIF_ATCMD_WIFI_ETHERNET_TCP_CONNECT           "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n"
#define NETIF_ATCMD_WIFI_ETHERNET_TCP_DISCONNECT        "AT+CIPCLOSE=%d\r\n"
#define NETIF_ATCMD_WIFI_ETHERNET_TCP_SEND              "AT+CIPSEND=%d,%d\r\n"
#define NETIF_ATCMD_WIFI_ETHERNET_TCP_RECV_DATA         "AT+CIPRECVDATA=%d,%d\r\n"






#endif // NETIF_ATCMD_TCP_H