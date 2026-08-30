#ifndef NETIF_HTTP_H
#define NETIF_HTTP_H

#include "stdbool.h"
#include "stdint.h"
#include "netif_def.h"
#include <netif_opts.h>

#if(NETIF_USE_HTTP == 1)
typedef enum
{
	NETIF_HTTP_CONTENT_TYPE_APPLICATION_XWWWFORMURLENCODED = 0,
	NETIF_HTTP_CONTENT_TYPE_APPLICATION_JSON = 1,
	NETIF_HTTP_CONTENT_TYPE_MULTIPARTFORMDATA = 2,
	NETIF_HTTP_CONTENT_TYPE_TEXTXML = 3,
} netif_http_content_type_t;

typedef enum
{
	NETIF_HTTP_TRANSPORT_TYPE_OVER_TCP = 1,
	NETIF_HTTP_TRANSPORT_TYPE_OVER_SSL = 2,
} netif_http_transport_type_t;

typedef struct
{
	netif_http_content_type_t content_type;
	char* url;
	netif_http_content_type_t transport_type;
	char* data;
	char** req_header;
	// Callback Function
	void (*on_post)(uint8_t result);
	void (*on_get)(uint8_t result, char* data, uint16_t data_len);
	void (*on_put)(uint8_t result);
} netif_http_request_t;

/**
 * @brief Initialize HTTP Apps
 *
 * @return true if OK
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_init();

/**
 * @brief Run HTTP Stack in Super Loop, handle event, ...
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_run();

/**
 * @brief Deinitialize HTTP Apps
 *
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_deinit();

// Specific Function
/**
 * @brief Send HTTP GET request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_get_request(netif_http_request_t* request);

/**
 * @brief Send HTTP POST request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_post_request(netif_http_request_t* request);

/**
 * @brief Send HTTP PUT request
 *
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_put_request(netif_http_request_t* request);

// --- Chunked download API (4G only) -----------------------------------
// Purpose-built for pulling a large binary body (e.g. an OTA image) in
// caller-controlled chunks, unlike netif_http_request_t's whole-body
// callback shape above. All three are non-blocking state machines: call
// repeatedly until they return something other than NETIF_IN_PROCESS.

typedef struct
{
	char* url;
	uint32_t contentLength; // filled in by netif_http_download_start() once the GET completes
} netif_http_download_t;

/**
 * @brief Start a GET download: HTTPINIT + PARAM(CID,URL) + ACTION(GET).
 *        Fills download->contentLength once the module reports completion.
 */
netif_status_t netif_http_download_start(netif_http_download_t* download);

/**
 * @brief Read `len` bytes of the downloaded body starting at `offset` into
 *        `data` (caller-owned, must be >= len). Sets *readLen on success.
 */
netif_status_t netif_http_download_read(netif_http_download_t* download, uint32_t offset,
										  uint8_t* data, uint32_t len, uint32_t* readLen);

/**
 * @brief Terminate the HTTP session (HTTPTERM).
 */
netif_status_t netif_http_download_stop(netif_http_download_t* download);

#endif

#endif