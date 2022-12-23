#ifndef NETIF_ATCMD_WIFI_H
#define NETIF_ATCMD_WIFI_H

// Enable Station Mode (1) and Enable Autoreconnect (1)
#define NETIF_ATCMD_WIFI_STATION_MODE "AT+CWMODE=1,1\n"
// Connect to AP using ssid, password
#define NETIF_ATCMD_WIFI_CONNECT_AP "AT+CWJAP=%s,%s\n"
// Disconnect from AP
#define NETIF_ATCMD_WIFI_DISCONNECT_AP "AT+CWQAP\n"
// Check Wifi State
#define NETIF_ATCMD_WIFI_GET_STATE  "AT+CWSTATE?\n"
// Start Smartconfig
#define NETIF_ATCMD_WIFI_START_SMARTCONFIG "AT+CWSTARTSMART\n"
// Stop Smartconfig
#define NETIF_ATCMD_WIFI_STOP_SMARTCONFIG   "AT+CWSTOPSMART\n"
// Get IP Address
#define NETIF_ATCMD_WIFI_GET_IP         "AT+CIPSTA?\n"
// Get Mac Address
#define NETIF_ATCMD_WIFI_GET_MAC        "AT+CIPSTAMAC?\n"
#endif //NETIF_ATCMD_WIFI_H
