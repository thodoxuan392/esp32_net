#ifndef NETIF_ATCMD_4G_H
#define NETIF_ATCMD_4G_H

/******************************Checking Command ************************/
// Testing
#define NETIF_ATCMD_4G_TESTTING "AT\r\n"
// Software Reset
#define NETIF_ATCMD_4G_RESET "AT+CRESET\r\n"
// Enable Echo
#define NETIF_ATCMD_4G_ECHO_ENABLE "ATE1\r\n"
// Enable Echo
#define NETIF_ATCMD_4G_ECHO_DISABLE "ATE0\r\n"
// Check Network Registration
#define NETIF_ATCMD_4G_NETWORK_REGIS "AT+CREG\r\n"
// Packet domain attach or detach
#define NETIF_ATCMD_4G_CGATT "AT+CGATT=1\r\n"
// ShowPDPaddress
#define NETIF_ATCMD_4G_CGPADDR "AT+CGPADDR=1\r\n"
// Define the PDP Context
#define NETIF_ATCMD_4G_PDP_CONTEXT "AT+CGDCONT=1,\"IP\",\"v-internet\"\r\n"
// Socket configuration
#define NETIF_ATCMD_4G_SOCKET_CONFIG "AT+CGSOCKCONT=1,\"IP\",\"cmet\"\r\n"
#define NETIF_ATCMD_4G_CSOCKSETPN "AT+CSOCKSETPN=1\r\n"
// Disable TCP/IP Mode
#define NETIF_ATCMD_4G_NON_TRANSPARENT_TCP_MODE "AT+CIPMODE=0\r\n"
// Disable TCP/IP Mode
#define NETIF_ATCMD_4G_NETOPEN "AT+NETOPEN\r\n"
// Get IMEI Address
#define NETIF_ATCMD_4G_GET_IMEI "AT+SIMEI?\r\n"
// Set Rx polling Mode
#define NETIF_ATCMD_4G_TCP_RX_POLLING_MODE "AT+CIPRXGET=1\r\n"
// Get Imsi
#define NETIF_ATCMD_4G_GET_IMSI "AT+CIMI\r\n"
// Get Iccid
#define NETIF_ATCMD_4G_GET_ICCID "AT+CICCID\r\n"


// Get signal strength
#define NETIF_ATCMD_4G_CSQ "AT+CSQ\r\n"

// Set auto response signal quality
#define NETIF_ATCMD_4G_AUTO_CSQ "AT+AUTOCSQ=1,0\r\n"
// Get TCP/IP timeout value
#define NETIF_ATCMD_4G_GET_CIPTIMEOUT "AT+CIPTIMEOUT?\r\n"

#endif // NETIF_ATCMD_ETHERNET_H
