#ifndef NETIF_ATCMD_WIFI_H
#define NETIF_ATCMD_WIFI_H

// Enable Station Mode (1) and Enable Autoreconnect (1)
#define NETIF_ATCMD_WIFI_STATION_MODE "AT+CWMODE=1,1\r\n"
// Configurate AutoReconnect with 100 max times
#define NETIF_ATCMD_WIFI_RECONNECT_CONFIG "AT+CWRECONNCFG=5,100\r\n"
// Reset wifi
#define NETIF_ATCMD_WIFI_RESET 	"AT+RST\r\n"
// Wifi Reconnect
#define NETIF_ATCMD_WIFI_RECONNECT "AT+CWJAP\r\n"
// Connect to AP using ssid, password
#define NETIF_ATCMD_WIFI_CONNECT_AP "AT+CWJAP=%s,%s\r\n"
// Disconnect from AP
#define NETIF_ATCMD_WIFI_DISCONNECT_AP "AT+CWQAP\r\n"
// Check Wifi State
#define NETIF_ATCMD_WIFI_GET_STATE  "AT+CWSTATE?\r\n"
// Start Smartconfig
#define NETIF_ATCMD_WIFI_START_SMARTCONFIG "AT+CWSTARTSMART\r\n"
// Stop Smartconfig
#define NETIF_ATCMD_WIFI_STOP_SMARTCONFIG   "AT+CWSTOPSMART\r\n"
// Get IP Address
#define NETIF_ATCMD_WIFI_GET_IP         "AT+CIPSTA?\r\n"
// Get Mac Address
#define NETIF_ATCMD_WIFI_GET_MAC        "AT+CIPSTAMAC?\r\n"
#endif //NETIF_ATCMD_WIFI_H
