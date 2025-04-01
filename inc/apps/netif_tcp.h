#ifndef NETIF_TCP_H
#define NETIF_TCP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <netif_def.h>
#include <netif_opts.h>

#if(NETIF_USE_TCP == 1)

typedef struct
{
	uint8_t clientNo;
	int clientPort;

	char* serverIp;
	uint16_t serverPort;

	bool loopDisable;

	void (*on_receive_indication)(void);
} netif_tcp_client_t;

netif_status_t netif_tcp_init();
netif_status_t netif_tcp_run();
netif_status_t netif_tcp_deinit();
netif_status_t netif_tcp_start(netif_tcp_client_t* client);
netif_status_t netif_tcp_stop(netif_tcp_client_t* client);
netif_status_t netif_tcp_connect(netif_tcp_client_t* client);
netif_status_t netif_tcp_disconnect(netif_tcp_client_t* client);
netif_status_t netif_tcp_send(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength);
netif_status_t netif_tcp_receive(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength,
								 uint32_t* readLength);

#endif
#endif // NETIF_TCP_H
