#ifndef NETIF_HTTP_H
#define NETIF_HTTP_H

#include "stdbool.h"
#include "stdint.h"

typedef enum {
    NETIF_HTTP_OPT_GET = 2,
    NETIF_HTTP_OPT_POST =3,
    NETIF_HTTP_OPT_PUT = 4
}netif_http_opt_t;

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
    netif_http_opt_t opt;
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
 * @return false if Fail or Timeout
 */
bool netif_http_init();

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool netif_http_run();


/**
 * @brief Deinitialize HTTP Apps 
 * 
 * @return true if OK
 * @return false if Fail or Timeout
 */
bool netif_http_deinit();

// Specific Function
/**
 * @brief Send HTTP request, support GET, POST, PUT
 * 
 * @param request 
 * @return true if OK
 * @return false if fail or timeout
 */
bool netif_http_send_request(netif_http_request_t * request);


#endif