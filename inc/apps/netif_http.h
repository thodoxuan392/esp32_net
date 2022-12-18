#ifndef NETIF_HTTP_H
#define NETIF_HTTP_H

#include "stdbool.h"
#include "stdint.h"
#include "netif_def.h"

typedef enum{
    NETIF_HTTP_CONTENT_TYPE_APPLICATION_XWWWFORMURLENCODED = 0,
    NETIF_HTTP_CONTENT_TYPE_APPLICATION_JSON = 1,
    NETIF_HTTP_CONTENT_TYPE_MULTIPARTFORMDATA = 2 ,
    NETIF_HTTP_CONTENT_TYPE_TEXTXML = 3,
}netif_http_content_type_t;

typedef enum{
    NETIF_HTTP_TRANSPORT_TYPE_OVER_TCP = 1,
    NETIF_HTTP_TRANSPORT_TYPE_OVER_SSL = 2,
}netif_http_transport_type_t;

typedef struct {
    netif_http_content_type_t content_type;
    char * url;
    netif_http_content_type_t transport_type;
    char *data;
    char **req_header;
    // Callback Function
    void (*on_post)(uint8_t result);
    void (*on_get)(uint8_t result, char * data, uint16_t data_len);
    void (*on_put)(uint8_t result);
}netif_http_request_t;

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
netif_status_t netif_http_send_get_request(netif_http_request_t * request);


/**
 * @brief Send HTTP POST request
 * 
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_post_request(netif_http_request_t * request);


/**
 * @brief Send HTTP PUT request
 * 
 * @param request HTTP request
 * @return netif_status_t Status of Process
 */
netif_status_t netif_http_send_put_request(netif_http_request_t * request);


#endif