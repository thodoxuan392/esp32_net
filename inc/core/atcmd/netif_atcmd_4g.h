#ifndef NETIF_ATCMD_4G_H
#define NETIF_ATCMD_4G_H


// Test Command
#define NETIF_CMD_4G_TEST     "AT\r\n"
/******************************Checking Command ************************/
// Enable Echo 
#define NETIF_CMD_4G_ECHO_ENABLE     "ATE1\r\n"
// Check Network Registration
#define NETIF_CMD_4G_NETWORK_REGIS     "AT+CREG?\r\n"
// Get IP Addr
#define NETIF_CMD_4G_NETWORK_REGIS     "AT+IPADDR\r\n"
// Get MAC Addr
#define NETIF_CMD_4G_NETWORK_REGIS     "AT+IPADDR\r\n"

#endif //NETIF_ATCMD_ETHERNET_H