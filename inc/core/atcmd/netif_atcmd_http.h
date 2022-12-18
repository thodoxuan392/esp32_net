#ifndef NETIF_ATCMD_HTTP_H
#define NETIF_ATCMD_HTTP_H

// Sent HTTP Get Request (url)
#define NETIF_ATCMD_HTTP_GET  "AT+HTTPCGET=%s\n"
// Sent HTTP Post Request (url,length)
#define NETIF_ATCMD_HTTP_POST "AT+HTTPCPOST=%s,%d\n"
// Sent HTTP Put Request (url,length)
#define NETIF_ATCMD_HTTP_PUT "AT+HTTPCPUT=%s,%d\n"

#endif //NET_ATCMD_HTTP_H