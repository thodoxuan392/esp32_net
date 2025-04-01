#include <apps/netif_tcp.h>
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_tcp.h"
#include "manager/netif_manager.h"
#include "netif_opts.h"
#include "utils_buffer.h"
#include "utils_logger.h"
#include "utils_string.h"

#if(NETIF_USE_TCP == 1)

// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];
static netif_tcp_client_t* tcp_client;

// Request State
enum
{
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
	// Wifi Ethernet Request State
	STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT,
	STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT,
	STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND,
	STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND_INPUT,
	STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA,
	STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE,
	#endif

	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
	// 4G Request State
	STATE_4G_TCP_START,
	STATE_4G_TCP_STOP,
	STATE_4G_TCP_CONNECT,
	STATE_4G_TCP_DISCONNECT,
	STATE_4G_TCP_SETUP_RX_MODE,
	STATE_4G_TCP_SEND,
	STATE_4G_TCP_SEND_INPUT,
	STATE_4G_TCP_RX_GET,
	STATE_4G_TCP_WAIT_FOR_RESPONSE,
	#endif
};

	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
static netif_status_t netif_wifi_ethernet_tcp_start(netif_tcp_client_t* client);
static netif_status_t netif_wifi_ethernet_tcp_stop(netif_tcp_client_t* client);
static netif_status_t netif_wifi_ethernet_tcp_connect(netif_tcp_client_t* client);
static netif_status_t netif_wifi_ethernet_tcp_disconnect(netif_tcp_client_t* client);
static netif_status_t netif_wifi_ethernet_tcp_send(netif_tcp_client_t* client, uint8_t* data,
												   uint32_t dataLength);

static netif_status_t netif_wifi_ethernet_tcp_receive(netif_tcp_client_t* client, uint8_t* data,
													  uint32_t dataLength, uint32_t* readLength);
static netif_status_t netif_wifi_ethernet_tcp_parse_on_receive(netif_tcp_client_t* client,
															   uint8_t* data, uint32_t dataLength,
															   uint32_t* readLength);
	#endif

	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
static netif_status_t netif_4g_tcp_start(netif_tcp_client_t* client);
static netif_status_t netif_4g_tcp_stop(netif_tcp_client_t* client);
static netif_status_t netif_4g_tcp_connect(netif_tcp_client_t* client);
static netif_status_t netif_4g_tcp_disconnect(netif_tcp_client_t* client);
static netif_status_t netif_4g_tcp_send(netif_tcp_client_t* client, uint8_t* data,
										uint32_t dataLength);
static netif_status_t netif_4g_tcp_receive(netif_tcp_client_t* client, uint8_t* data,
										   uint32_t dataLength, uint32_t* readLength);
static netif_status_t netif_4g_tcp_parse_on_start(netif_tcp_client_t* client, uint8_t* errorCode);
static netif_status_t netif_4g_tcp_parse_on_stop(netif_tcp_client_t* client, uint8_t* errorCode);
static netif_status_t netif_4g_tcp_parse_on_connect(netif_tcp_client_t* client, uint8_t* linkNo,
													uint8_t* errorCode);
static netif_status_t netif_4g_tcp_parse_on_disconnect(netif_tcp_client_t* client, uint8_t* linkNo,
													   uint8_t* errorCode);
static netif_status_t netif_4g_tcp_parse_on_send(netif_tcp_client_t* client, uint8_t* linkNo,
												 uint32_t* reqSendLength, uint32_t* cnfSendLength);
static netif_status_t netif_4g_tcp_parse_on_receive(netif_tcp_client_t* client, uint8_t* data,
													uint32_t dataLength, uint32_t* readLength);
	#endif

netif_status_t netif_tcp_init()
{
	return NETIF_OK;
}
netif_status_t netif_tcp_run()
{
	netif_core_response_t at_response;
	uint8_t* data;
	size_t data_len;

	// Disable loop
	if(tcp_client->loopDisable)
	{
		return NETIF_OK;
	}
	// Wait Connect AP Response
	if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response) ||
	   netif_core_atcmd_is_responded(NETIF_4G, &at_response))
	{
		switch(at_response)
		{
			case NETIF_4G_REPORT_TCP_CIP_RX_GET:
				// Donot use data from response -> Clean Core Buffer
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
				netif_core_atcmd_reset(NETIF_4G, false);
	#endif
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
	#endif
				if(tcp_client && tcp_client->on_receive_indication)
				{
					tcp_client->on_receive_indication();
				}
				break;
		}
	}
	return NETIF_OK;
}
netif_status_t netif_tcp_deinit()
{
	return NETIF_OK;
}
netif_status_t netif_tcp_start(netif_tcp_client_t* client)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	tcp_client = client;
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_start(client);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_start(client);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}
netif_status_t netif_tcp_stop(netif_tcp_client_t* client)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_stop(client);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_stop(client);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}
netif_status_t netif_tcp_connect(netif_tcp_client_t* client)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_connect(client);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_connect(client);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}
netif_status_t netif_tcp_disconnect(netif_tcp_client_t* client)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_disconnect(client);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_disconnect(client);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}
netif_status_t netif_tcp_send(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_send(client, data, dataLength);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_send(client, data, dataLength);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}
netif_status_t netif_tcp_receive(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength,
								 uint32_t* readLength)
{
	netif_manager_mode_t netmanager_mode = netif_manager_get_mode();
	switch(netmanager_mode)
	{
		case NETIF_MANAGER_WIFI_MODE:
	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
		case NETIF_MANAGER_ETHERNET_MODE:
			return netif_wifi_ethernet_tcp_receive(client, data, dataLength, readLength);
			break;
	#endif
	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
		case NETIF_MANAGER_4G_MODE:
			return netif_4g_tcp_receive(client, data, dataLength, readLength);
			break;
	#endif
		default:
			// If not in above mode -> fail
			return NETIF_FAIL;
			break;
	}
}

	#if defined(NETIF_WIFI_ETHERNET_ENABLE) && NETIF_WIFI_ETHERNET_ENABLE == 1
netif_status_t netif_wifi_ethernet_tcp_start(netif_tcp_client_t* client)
{
	return NETIF_OK;
}
netif_status_t netif_wifi_ethernet_tcp_stop(netif_tcp_client_t* client)
{
	return NETIF_OK;
}
netif_status_t netif_wifi_ethernet_tcp_connect(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch(state)
	{
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
				// Send Client Config to AP to Wifi Module
				size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_TCP_CONNECT, client->clientNo,
							   client->serverIp, client->serverPort);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT;
				utils_log_error("Tcp connect timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response))
			{
				if((response == NETIF_RESPONSE_OK) || (response == NETIF_RESPONSE_ERROR))
				{ // Force Error is Success because don't have read command to get client status
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_WIFI_ETHERNET_REPORT_BUSY)
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_CONNECT;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}
netif_status_t netif_wifi_ethernet_tcp_disconnect(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch(state)
	{
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
				// Send Client Config to AP to Wifi Module
				size =
					sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_TCP_DISCONNECT, client->clientNo);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
				utils_log_error("Tcp connect timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response))
			{
				if((response == NETIF_RESPONSE_OK) || (response == NETIF_RESPONSE_ERROR))
				{ // Force Error is Success because don't have read command to get client status
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
					return NETIF_OK;
				}
				else if(response == NETIF_WIFI_ETHERNET_REPORT_BUSY)
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}
netif_status_t netif_wifi_ethernet_tcp_send(netif_tcp_client_t* client, uint8_t* data,
											uint32_t dataLength)
{
	static uint8_t state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch(state)
	{
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
				// Send Client Config to AP to Wifi Module
				size =
					sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_TCP_DISCONNECT, client->clientNo);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND_INPUT;
			}
			break;
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND_INPUT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
				utils_log_error("Mqtt publish timeout to receive input\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response))
			{
				if(response == NETIF_RESPONSE_INPUT)
				{
					last_time_sent = NETIF_GET_TIME_MS();
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
					netif_core_wifi_ethernet_output(data, dataLength);
					retry = 0;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE;
				}
				else if(response == NETIF_RESPONSE_ERROR ||
						response == NETIF_WIFI_ETHERNET_REPORT_BUSY)
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						// Reset Buffer and Indication if Try number over
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
				}
				// Ignore other case
				else
				{
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
				}
			}
			break;
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
				utils_log_error("Tcp connect timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{ // Force Error is Success because don't have read command to get client status
					netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
					retry = 0;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
					return NETIF_OK;
				}
				else if((response == NETIF_WIFI_ETHERNET_REPORT_BUSY) ||
						(response == NETIF_RESPONSE_ERROR) ||
						(response == NETIF_WIFI_ETHERNET_RESPONSE_SEND_FAIL))
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_SEND;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}
netif_status_t netif_wifi_ethernet_tcp_receive(netif_tcp_client_t* client, uint8_t* data,
											   uint32_t dataLength, uint32_t* readLength)
{
	static uint8_t state = STATE_WIFI_ETHERNET_ETHERNET_TCP_DISCONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	switch(state)
	{
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
				// Send Client Config to AP to Wifi Module
				size =
					sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_TCP_DISCONNECT, client->clientNo);
				netif_core_wifi_ethernet_output(at_message, size);
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_WIFI_ETHERNET_ETHERNET_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				state = STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA;
				utils_log_error("Tcp receive data timeout to receive ok\r\n");
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &response))
			{
				if(response == NETIF_WIFI_ETHERNET_REPORT_TCP_CIP_RECV_DATA)
				{
					if(netif_wifi_ethernet_tcp_parse_on_receive(client, data, dataLength,
																readLength) == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, true);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA;
						return NETIF_OK;
					}
				}
				else if((response == NETIF_WIFI_ETHERNET_REPORT_BUSY) ||
						(response == NETIF_RESPONSE_ERROR))
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
						retry = 0;
						state = STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_WIFI_ETHERNET_ETHERNET_TCP_RECV_DATA;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

static netif_status_t netif_wifi_ethernet_tcp_parse_on_receive(netif_tcp_client_t* client,
															   uint8_t* data, uint32_t dataLength,
															   uint32_t* readLength)
{
}
	#endif

	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1
netif_status_t netif_4g_tcp_start(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_4G_TCP_START;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint8_t errorCode;
	switch(state)
	{
		case STATE_4G_TCP_START:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_START_SOCKET_SERVICE);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_START;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_START;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{
					// Ignore OK response // wait until get NETOPEN
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_4G_REPORT_TCP_NET_OPEN)
				{
					if(netif_4g_tcp_parse_on_start(tcp_client, &errorCode) == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						state = STATE_4G_TCP_START;
						if(errorCode == 0)
						{
							retry = 0;
							return NETIF_OK;
						}
						else
						{
							// Retry
							if(retry >= NETIF_MAX_RETRY)
							{
								retry = 0;
								return NETIF_FAIL;
							}
							retry++;
						}
					}
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					// Retry
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_TCP_START;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_TCP_START;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
netif_status_t netif_4g_tcp_stop(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_4G_TCP_STOP;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint8_t errorCode;
	switch(state)
	{
		case STATE_4G_TCP_STOP:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_STOP_SOCKET_SERVICE);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_STOP;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_STOP;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_4G_REPORT_TCP_NET_CLOSE)
				{
					if(netif_4g_tcp_parse_on_stop(tcp_client, &errorCode) == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						state = STATE_4G_TCP_STOP;
						if(errorCode == 0)
						{
							retry = 0;
							return NETIF_OK;
						}
						else
						{
							// Retry
							if(retry >= NETIF_MAX_RETRY)
							{
								retry = 0;
								return NETIF_FAIL;
							}
							retry++;
						}
					}
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_TCP_STOP;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_TCP_STOP;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
netif_status_t netif_4g_tcp_connect(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_4G_TCP_CONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint8_t linkNo, errorCode;
	switch(state)
	{
		case STATE_4G_TCP_CONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_CONNECT, client->clientNo,
							   client->serverIp, client->serverPort, client->clientPort);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_CONNECT;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_CONNECT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_4G_REPORT_TCP_CIP_OPEN)
				{
					if(netif_4g_tcp_parse_on_connect(tcp_client, &linkNo, &errorCode) == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						state = STATE_4G_TCP_CONNECT;
						if(errorCode == 0)
						{
							retry = 0;
							return NETIF_OK;
						}
						else
						{
							// Retry
							if(retry >= NETIF_MAX_RETRY)
							{
								retry = 0;
								return NETIF_FAIL;
							}
							retry++;
						}
					}
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_TCP_CONNECT;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_TCP_CONNECT;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
netif_status_t netif_4g_tcp_disconnect(netif_tcp_client_t* client)
{
	static uint8_t state = STATE_4G_TCP_DISCONNECT;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint8_t linkNo, errorCode;
	switch(state)
	{
		case STATE_4G_TCP_DISCONNECT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_DISCONNECT, client->clientNo);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_DISCONNECT;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_DISCONNECT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_4G_REPORT_TCP_CIP_CLOSE)
				{
					if(netif_4g_tcp_parse_on_disconnect(tcp_client, &linkNo, &errorCode) ==
					   NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						state = STATE_4G_TCP_DISCONNECT;
						if(errorCode == 0)
						{
							retry = 0;
							return NETIF_OK;
						}
						else
						{
							// Retry
							if(retry >= NETIF_MAX_RETRY)
							{
								retry = 0;
								return NETIF_FAIL;
							}
							retry++;
						}
					}
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_TCP_DISCONNECT;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_TCP_DISCONNECT;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
netif_status_t netif_4g_tcp_send(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength)
{
	static uint8_t state = STATE_4G_TCP_SEND;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint8_t linkNo;
	uint32_t reqSendLength, cnfSendLength;
	switch(state)
	{
		case STATE_4G_TCP_SEND:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_SEND, client->clientNo, dataLength);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_SEND_INPUT;
			}
			break;
		case STATE_4G_TCP_SEND_INPUT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				if(retry >= NETIF_MAX_RETRY)
				{
					utils_log_error("TCP send data response timeout, maximum retried time\r\n");
					// Reset Buffer and Indication if Try number over
					retry = 0;
					state = STATE_4G_TCP_SEND;
					return NETIF_FAIL;
				}
				utils_log_warn("TCP send data response failed, retrying ...\r\n");
				retry++;
				state = STATE_4G_TCP_SEND;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_INPUT)
				{
					netif_core_atcmd_reset(NETIF_4G, true);
					last_time_sent = NETIF_GET_TIME_MS();
					utils_log_debug("%.*s", dataLength, data);
					netif_core_4g_output(data, dataLength);
					retry = 0;
					state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					utils_log_error("TCP send data response failed, maximum retried time\r\n");
					netif_core_atcmd_reset(NETIF_4G, true);
					if(retry >= NETIF_MAX_RETRY)
					{
						// Reset Buffer and Indication if Try number over
						retry = 0;
						state = STATE_4G_TCP_SEND;
						return NETIF_FAIL;
					}
					utils_log_error("MQTT publish topic input got error, retrying ...\r\n");
					retry++;
					state = STATE_4G_TCP_SEND;
				}
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_SEND;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_SEND;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_RESPONSE_OK)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_4G_REPORT_TCP_CIP_SEND)
				{
					if(netif_4g_tcp_parse_on_send(tcp_client, &linkNo, &reqSendLength,
												  &cnfSendLength) == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						state = STATE_4G_TCP_SEND;
						retry = 0;
						return NETIF_OK;
					}
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_TCP_SEND;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_TCP_SEND;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
netif_status_t netif_4g_tcp_receive(netif_tcp_client_t* client, uint8_t* data, uint32_t dataLength,
									uint32_t* readLength)
{
	static uint8_t state = STATE_4G_TCP_RX_GET;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;

	// Lock TCP Loop Process for RX Indication
	client->loopDisable = true;

	switch(state)
	{
		case STATE_4G_TCP_RX_GET:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				// Clear Before Data
				netif_core_atcmd_reset(NETIF_4G, true);
				// Send Connect Command to 4G Module
				size = sprintf(at_message, NETIF_ATCMD_4G_TCP_READ_DATA_WITH_LENGTH,
							   client->clientNo, dataLength);
				utils_log_debug(at_message);
				netif_core_4g_output(at_message, size);
				state = STATE_4G_TCP_WAIT_FOR_RESPONSE;
			}
			break;
		case STATE_4G_TCP_WAIT_FOR_RESPONSE:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, false);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_TCP_RX_GET;
					client->loopDisable = false;
					return NETIF_FAIL;
				}
				retry++;
				state = STATE_4G_TCP_RX_GET;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_4G_REPORT_TCP_CIP_RX_GET)
				{
					if(netif_4g_tcp_parse_on_receive(client, data, dataLength, readLength) ==
					   NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						retry = 0;
						state = STATE_4G_TCP_RX_GET;
						client->loopDisable = false;
						return NETIF_OK;
					}
				}
				else if(response == NETIF_4G_REPORT_TCP_CIP_RX_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, false);
					retry = 0;
					state = STATE_4G_TCP_RX_GET;
					client->loopDisable = false;
					return NETIF_FAIL;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_start(netif_tcp_client_t* client, uint8_t* errorCode)
{
	static uint8_t onStartBuffer[20];
	static uint32_t onStartBufferLength;

	char* outputBuffer[1];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onStartBuffer[onStartBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onStartBuffer, onStartBufferLength, ",", outputBuffer,
											1, "\r\n"))
		{
			*errorCode = (uint8_t)utils_string_to_int(outputBuffer[0], strlen(outputBuffer[0]));

			onStartBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_stop(netif_tcp_client_t* client, uint8_t* errorCode)
{
	static uint8_t onStopBuffer[20];
	static uint32_t onStopBufferLength;

	char* outputBuffer[1];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onStopBuffer[onStopBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onStopBuffer, onStopBufferLength, ",", outputBuffer, 1,
											"\r\n"))
		{
			*errorCode = (uint8_t)utils_string_to_int(outputBuffer[0], strlen(outputBuffer[0]));

			onStopBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_connect(netif_tcp_client_t* client, uint8_t* linkNo,
													uint8_t* errorCode)
{
	static uint8_t onConnectBuffer[30];
	static uint32_t onConnectBufferLength = 0;

	char* outputBuffer[2];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onConnectBuffer[onConnectBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onConnectBuffer, onConnectBufferLength, ",",
											outputBuffer, 2, "\r\n"))
		{
			*linkNo = (uint8_t)utils_string_to_int(outputBuffer[0], strlen(outputBuffer[0]));
			*errorCode = (uint8_t)utils_string_to_int(outputBuffer[1], strlen(outputBuffer[1]));

			onConnectBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_disconnect(netif_tcp_client_t* client, uint8_t* linkNo,
													   uint8_t* errorCode)
{
	static uint8_t onDisconnectBuffer[30];
	static uint32_t onDisconnectBufferLength = 0;

	char* outputBuffer[2];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onDisconnectBuffer[onDisconnectBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onDisconnectBuffer, onDisconnectBufferLength, ",",
											outputBuffer, 2, "\r\n"))
		{
			*linkNo = (uint8_t)utils_string_to_int(outputBuffer[0], strlen(outputBuffer[0]));
			*errorCode = (uint8_t)utils_string_to_int(outputBuffer[1], strlen(outputBuffer[1]));

			onDisconnectBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_send(netif_tcp_client_t* client, uint8_t* linkNo,
												 uint32_t* reqSendLength, uint32_t* cnfSendLength)
{
	static uint8_t onSendBuffer[30];
	static uint32_t onSendBufferLength = 0;

	char* outputBuffer[3];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onSendBuffer[onSendBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onSendBuffer, onSendBufferLength, ",", outputBuffer, 3,
											"\r\n"))
		{
			*linkNo = (uint8_t)utils_string_to_int(outputBuffer[0], strlen(outputBuffer[0]));
			*reqSendLength = utils_string_to_int(outputBuffer[1], strlen(outputBuffer[1]));
			*cnfSendLength = utils_string_to_int(outputBuffer[2], strlen(outputBuffer[2]));

			onSendBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
static netif_status_t netif_4g_tcp_parse_on_receive(netif_tcp_client_t* client, uint8_t* data,
													uint32_t dataLength, uint32_t* readLength)
{
	static uint8_t onReceiveBuffer[4096];
	static uint32_t onReceiveBufferLength = 0;

	if(dataLength >= 4096)
	{
		utils_log_error("[NETIF_4G_TCP] Data length %d is over maximum 4096\r\n", dataLength);
		return NETIF_FAIL;
	}

	char* outputBuffer[4];
	char* outputBuffer2[2];

	if(netif_core_atcmd_get_data_after(NETIF_4G, &onReceiveBuffer[onReceiveBufferLength++]))
	{
		if(utils_string_split_with_fixed_no(onReceiveBuffer, onReceiveBufferLength, ",",
											outputBuffer, 4, "OK"))
		{
			*readLength = utils_string_to_int(outputBuffer[2], strlen(outputBuffer[2]));

			if(utils_string_split_with_fixed_no(outputBuffer[3],
												(uint32_t)onReceiveBuffer + onReceiveBufferLength -
													(uint32_t)outputBuffer[3],
												"\r\n", outputBuffer2, 2, "OK"))
			{
				strncpy(data, outputBuffer2[1], *readLength);
			}
			onReceiveBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}
	#endif

#endif
