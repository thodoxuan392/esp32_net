#include "apps/netif_http.h"
#include "core/netif_core.h"
#include "core/atcmd/netif_atcmd_http.h"
#include "manager/netif_manager.h"
#include "netif_opts.h"
#include "utils_buffer.h"
#include "utils_logger.h"
#include "utils_string.h"

#if(NETIF_USE_HTTP == 1)

// AT Message Buffer
static char at_message[NETIF_ATCMD_BUFFER_SIZE];

/**
 * @brief Initialize HTTP Apps
 *
 * @return true if OK
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_init()
{
	utils_log_debug("Netif Http Init\r\n");
	// Do nothing
	return NETIF_OK;
}

/**
 * @brief Run HTTP Stack in Super Loop, handle event, ...
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_run()
{
	// Do nothing
}

/**
 * @brief Deinitialize HTTP Apps
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_deinit()
{
	utils_log_info("Netif Http Deinit\r\n");
	// Do nothing
	return NETIF_OK;
}

// Specific Function
/**
 * @brief Send HTTP GET request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_get_request(netif_http_request_t* request)
{
	static uint8_t step = 0;
	netif_core_response_t at_response;
	int size;
	switch(step)
	{
		case 0:
			// Send Connect to AP to Wifi Module
			size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_HTTP_GET, request->url);
			netif_core_wifi_ethernet_output(at_message, size);
			// Switch wait to Connect AP Response
			step = 1;
			break;
		case 1:
			// Wait Connect AP Response
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response))
			{
				// Reset step
				step = 0;
				// Donot use data from response -> Clean Core Buffer
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
				// Check AT Response
				if(at_response == NETIF_RESPONSE_OK)
				{
					// Switch to Send Post data
					return NETIF_OK;
				}
				else
				{
					return NETIF_FAIL;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

/**
 * @brief Send HTTP POST request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_post_request(netif_http_request_t* request)
{
	static uint8_t step = 0;
	netif_core_response_t at_response;
	int size;
	switch(step)
	{
		case 0:
			// Send Connect to AP to Wifi Module
			size = sprintf(at_message, NETIF_ATCMD_WIFI_ETHERNET_HTTP_POST, request->url,
						   strlen(request->data));
			netif_core_wifi_ethernet_output(at_message, size);
			// Switch wait to response
			step = 1;
			break;
		case 1:
			// Wait HTTP Post Response
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response))
			{
				// Donot use data from response -> Clean Core Buffer
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
				// Check AT Response
				if(at_response == NETIF_RESPONSE_OK)
				{
					// Wait for receive input indicator
					step = 2;
				}
				else if(at_response == NETIF_RESPONSE_OK)
				{
					// Reset step if fail
					step = 0;
					return NETIF_FAIL;
				}
			}
			break;
		case 2:
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response))
			{
				// Donot use data from response -> Clean Core Buffer
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
				// Check AT Response
				if(at_response == NETIF_RESPONSE_INPUT)
				{
					// Send Post data
					step = 3;
				}
				else
				{
					// Reset step if fail
					step = 0;
					return NETIF_FAIL;
				}
			}
			break;
		case 3:
			// Send Post data to Server
			netif_core_wifi_ethernet_output(request->data, strlen(request->data));
			// Switch wait to response
			step = 4;
			break;
		case 4:
			// Wait HTTP Post Response
			if(netif_core_atcmd_is_responded(NETIF_WIFI_ETHERNET, &at_response))
			{
				// Donot use data from response -> Clean Core Buffer
				netif_core_atcmd_reset(NETIF_WIFI_ETHERNET, false);
				// Check AT Response
				if(at_response == NETIF_WIFI_ETHERNET_RESPONSE_SEND_OK)
				{
					// Reset step
					step = 0;
					return NETIF_OK;
				}
				else
				{
					return NETIF_FAIL;
				}
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

/**
 * @brief Send HTTP PUT request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_put_request(netif_http_request_t* request);

	#if defined(NETIF_4G_ENABLE) && NETIF_4G_ENABLE == 1

// --- 4G chunked download implementation --------------------------------
enum
{
	STATE_4G_HTTP_DL_SEND_INIT = 0,
	STATE_4G_HTTP_DL_SEND_PARAM_CID,
	STATE_4G_HTTP_DL_SEND_PARAM_URL,
	STATE_4G_HTTP_DL_SEND_ACTION,
	STATE_4G_HTTP_DL_WAIT_ACK, // generic "OK" ack wait, shared by the 4 SEND states above
	STATE_4G_HTTP_DL_WAIT_ACTION_REPORT, // the real "+HTTPACTION:" report (whole GET download)
};

enum
{
	STATE_4G_HTTP_READ_SEND = 0,
	STATE_4G_HTTP_READ_WAIT_REPORT,
};

enum
{
	STATE_4G_HTTP_TERM_SEND = 0,
	STATE_4G_HTTP_TERM_WAIT_ACK,
};

static netif_status_t netif_4g_http_parse_on_action(uint32_t* contentLength,
													 uint16_t* httpStatus);
static netif_status_t netif_4g_http_parse_on_read(uint8_t* data, uint32_t maxLen,
												   uint32_t* readLen);

netif_status_t netif_http_download_start(netif_http_download_t* download)
{
	static uint8_t state = STATE_4G_HTTP_DL_SEND_INIT;
	static uint8_t ackNextState = STATE_4G_HTTP_DL_SEND_PARAM_CID;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;
	uint32_t contentLength;
	uint16_t httpStatus;

	if(netif_manager_get_mode() != NETIF_MANAGER_4G_MODE)
	{
		return NETIF_FAIL;
	}

	switch(state)
	{
		case STATE_4G_HTTP_DL_SEND_INIT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_INIT);
				netif_core_4g_output((uint8_t*)at_message, size);
				ackNextState = STATE_4G_HTTP_DL_SEND_PARAM_CID;
				state = STATE_4G_HTTP_DL_WAIT_ACK;
			}
			break;
		case STATE_4G_HTTP_DL_SEND_PARAM_CID:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_PARAM_CID);
				netif_core_4g_output((uint8_t*)at_message, size);
				ackNextState = STATE_4G_HTTP_DL_SEND_PARAM_URL;
				state = STATE_4G_HTTP_DL_WAIT_ACK;
			}
			break;
		case STATE_4G_HTTP_DL_SEND_PARAM_URL:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_PARAM, download->url);
				netif_core_4g_output((uint8_t*)at_message, size);
				ackNextState = STATE_4G_HTTP_DL_SEND_ACTION;
				state = STATE_4G_HTTP_DL_WAIT_ACK;
			}
			break;
		case STATE_4G_HTTP_DL_SEND_ACTION:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_ACTION, 0); // 0 = GET
				netif_core_4g_output((uint8_t*)at_message, size);
				ackNextState = STATE_4G_HTTP_DL_WAIT_ACTION_REPORT;
				state = STATE_4G_HTTP_DL_WAIT_ACK;
			}
			break;
		case STATE_4G_HTTP_DL_WAIT_ACK:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_HTTP_DL_SEND_INIT;
					return NETIF_TIMEOUT;
				}
				retry++;
				state = STATE_4G_HTTP_DL_SEND_INIT;
				break;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				if(response == NETIF_RESPONSE_OK)
				{
					retry = 0;
					state = ackNextState;
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_HTTP_DL_SEND_INIT;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_HTTP_DL_SEND_INIT;
				}
			}
			break;
		case STATE_4G_HTTP_DL_WAIT_ACTION_REPORT:
			// The module is downloading the whole body internally here --
			// give it much longer than a normal AT command round trip.
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_HTTP_ACTION_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				retry = 0;
				state = STATE_4G_HTTP_DL_SEND_INIT;
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response) &&
			   response == NETIF_4G_REPORT_HTTP_ACTION)
			{
				netif_status_t parseStatus =
					netif_4g_http_parse_on_action(&contentLength, &httpStatus);
				if(parseStatus == NETIF_OK)
				{
					netif_core_atcmd_reset(NETIF_4G, true);
					retry = 0;
					state = STATE_4G_HTTP_DL_SEND_INIT;
					if(httpStatus != 200)
					{
						utils_log_error("[NETIF_4G_HTTP] GET failed, status %u\r\n", httpStatus);
						return NETIF_FAIL;
					}
					download->contentLength = contentLength;
					return NETIF_OK;
				}
				else if(parseStatus == NETIF_FAIL)
				{
					netif_core_atcmd_reset(NETIF_4G, true);
					retry = 0;
					state = STATE_4G_HTTP_DL_SEND_INIT;
					return NETIF_FAIL;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}

netif_status_t netif_http_download_read(netif_http_download_t* download, uint32_t offset,
										  uint8_t* data, uint32_t len, uint32_t* readLen)
{
	static uint8_t state = STATE_4G_HTTP_READ_SEND;
	static uint8_t retry = 0;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;

	(void)download;

	if(netif_manager_get_mode() != NETIF_MANAGER_4G_MODE)
	{
		return NETIF_FAIL;
	}

	switch(state)
	{
		case STATE_4G_HTTP_READ_SEND:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_READ, (unsigned long)offset,
							   (unsigned long)len);
				netif_core_4g_output((uint8_t*)at_message, size);
				state = STATE_4G_HTTP_READ_WAIT_REPORT;
			}
			break;
		case STATE_4G_HTTP_READ_WAIT_REPORT:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				if(retry >= NETIF_MAX_RETRY)
				{
					retry = 0;
					state = STATE_4G_HTTP_READ_SEND;
					return NETIF_TIMEOUT;
				}
				retry++;
				state = STATE_4G_HTTP_READ_SEND;
				break;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				if(response == NETIF_4G_REPORT_HTTP_READ)
				{
					netif_status_t parseStatus = netif_4g_http_parse_on_read(data, len, readLen);
					if(parseStatus == NETIF_OK)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						retry = 0;
						state = STATE_4G_HTTP_READ_SEND;
						return NETIF_OK;
					}
					else if(parseStatus == NETIF_FAIL)
					{
						netif_core_atcmd_reset(NETIF_4G, true);
						retry = 0;
						state = STATE_4G_HTTP_READ_SEND;
						return NETIF_FAIL;
					}
					// else still accumulating -- keep polling
				}
				else if(response == NETIF_RESPONSE_ERROR)
				{
					netif_core_atcmd_reset(NETIF_4G, true);
					if(retry >= NETIF_MAX_RETRY)
					{
						retry = 0;
						state = STATE_4G_HTTP_READ_SEND;
						return NETIF_FAIL;
					}
					retry++;
					state = STATE_4G_HTTP_READ_SEND;
				}
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}

netif_status_t netif_http_download_stop(netif_http_download_t* download)
{
	static uint8_t state = STATE_4G_HTTP_TERM_SEND;
	static uint32_t last_time_sent = 0;
	netif_core_response_t response;
	int size;

	(void)download;

	if(netif_manager_get_mode() != NETIF_MANAGER_4G_MODE)
	{
		return NETIF_FAIL;
	}

	switch(state)
	{
		case STATE_4G_HTTP_TERM_SEND:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_APPS_RETRY_INTERVAL)
			{
				last_time_sent = NETIF_GET_TIME_MS();
				netif_core_atcmd_reset(NETIF_4G, true);
				size = sprintf(at_message, NETIF_ATCMD_4G_HTTP_TERMINATE);
				netif_core_4g_output((uint8_t*)at_message, size);
				state = STATE_4G_HTTP_TERM_WAIT_ACK;
			}
			break;
		case STATE_4G_HTTP_TERM_WAIT_ACK:
			if(NETIF_GET_TIME_MS() - last_time_sent > NETIF_ATCMD_TIMEOUT)
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				state = STATE_4G_HTTP_TERM_SEND;
				return NETIF_TIMEOUT;
			}
			if(netif_core_atcmd_is_responded(NETIF_4G, &response))
			{
				netif_core_atcmd_reset(NETIF_4G, true);
				state = STATE_4G_HTTP_TERM_SEND;
				return (response == NETIF_RESPONSE_OK) ? NETIF_OK : NETIF_FAIL;
			}
			break;
		default:
			break;
	}

	return NETIF_IN_PROCESS;
}

static netif_status_t netif_4g_http_parse_on_action(uint32_t* contentLength,
													 uint16_t* httpStatus)
{
	static char onActionBuffer[32];
	static uint32_t onActionBufferLength = 0;
	char* outputBuffer[3];

	if(netif_core_atcmd_get_data_after(NETIF_4G,
									   (uint8_t*)&onActionBuffer[onActionBufferLength++]))
	{
		if(onActionBufferLength >= sizeof(onActionBuffer))
		{
			onActionBufferLength = 0;
			return NETIF_FAIL;
		}
		if(utils_string_split_with_fixed_no(onActionBuffer, onActionBufferLength, ",",
											outputBuffer, 3, "\r\n"))
		{
			*httpStatus = (uint16_t)utils_string_to_int(outputBuffer[1], strlen(outputBuffer[1]));
			*contentLength = utils_string_to_int(outputBuffer[2], strlen(outputBuffer[2]));
			onActionBufferLength = 0;
			return NETIF_OK;
		}
	}
	return NETIF_IN_PROCESS;
}

// Reads "+HTTPREAD:" 's remainder: " <len>\r\n<len bytes of raw data>". Called
// repeatedly (one input byte per call, like netif_core's other byte-wise
// parsers) until the declared length has been fully copied into `data`.
static netif_status_t netif_4g_http_parse_on_read(uint8_t* data, uint32_t maxLen,
												   uint32_t* readLen)
{
	static char lenBuffer[12];
	static uint8_t lenBufferIndex = 0;
	static uint32_t parsedLen = 0;
	static uint32_t copiedLen = 0;
	static uint8_t state = 0; // 0=length digits, 1=skip LF, 2=copy payload
	uint8_t byte;

	if(!netif_core_atcmd_get_data_after(NETIF_4G, &byte))
	{
		return NETIF_IN_PROCESS;
	}

	switch(state)
	{
		case 0:
			if(byte == '\r')
			{
				parsedLen = utils_string_to_int(lenBuffer, lenBufferIndex);
				lenBufferIndex = 0;
				state = 1;
			}
			else if(byte >= '0' && byte <= '9' && lenBufferIndex < sizeof(lenBuffer))
			{
				lenBuffer[lenBufferIndex++] = (char)byte;
			}
			break;
		case 1:
			copiedLen = 0;
			state = 2;
			if(parsedLen > maxLen)
			{
				utils_log_error("[NETIF_4G_HTTP] Read length %lu exceeds buffer %lu\r\n",
								(unsigned long)parsedLen, (unsigned long)maxLen);
				state = 0;
				return NETIF_FAIL;
			}
			break;
		case 2:
			data[copiedLen++] = byte;
			if(copiedLen >= parsedLen)
			{
				*readLen = parsedLen;
				state = 0;
				return NETIF_OK;
			}
			break;
		default:
			break;
	}
	return NETIF_IN_PROCESS;
}

	#endif // NETIF_4G_ENABLE

#endif