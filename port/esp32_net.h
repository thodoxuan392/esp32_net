/*
 * esp32_net.h
 *
 *  Created on: Nov 16, 2022
 *      Author: xuanthodo
 */

#ifndef ESP32_NET_H_
#define ESP32_NET_H_

typedef enum {
	ESP32_NET_OK,
	ESP32_NET_ERR,
	ESP32_NET_TIMEOUT
}esp32_net_err_t;


typedef struct {

}esp32_net_config_t;

void esp32_net_init(esp32_net_config_t config);
void esp32_net_loop();

void esp32_net_input(uint8_t *data , uint16_t data_len);
void esp32_net_output()



#endif /* ESP32_NET_H_ */
