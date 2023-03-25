#ifndef NETIF_ATCMD_4G_H
#define NETIF_ATCMD_4G_H

/******************************Checking Command ************************/
// Testing
#define NETIF_ATCMD_4G_TESTTING     "AT\r\n"
// Software Reset
#define NETIF_ATCMD_4G_RESET		"AT+CRESET\r\n"
// Enable Echo 
#define NETIF_ATCMD_4G_ECHO_ENABLE     "ATE1\r\n"
// Check Network Registration
#define NETIF_ATCMD_4G_NETWORK_REGIS     "AT+CREG?\r\n"
// Define the PDP Context 
#define NETIF_ATCMD_4G_PDP_CONTEXT      "AT+CGDCONT=1,\"IP\",\"v-internet\"\r\n"
// Socket configuration 
#define NETIF_ATCMD_4G_SOCKET_CONFIG    "AT+CGSOCKCONT=1,\"IP\",\"cmet\"\r\n"
// Disable TCP/IP Mode  
#define NETIF_ATCMD_4G_DISABLE_TCP_MODE "AT+CIPMODE=0\r\n"
// Get IMEI Address
#define NETIF_ATCMD_4G_GET_IMEI         "AT+SIMEI?\r\n"



#endif //NETIF_ATCMD_ETHERNET_H
