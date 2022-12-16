ESP32_C_SOURCES_DIR = $(ESP32NET_DIR)/lwip/src \
$(ESP32NET_DIR)/port \
$(ESP32NET_DIR)/at_command
ESP32_C_SOURCES = $(addprefix $(ESP32NET_DIR)/", ${shell find  $(ESP32_C_SOURCES_DIR) -type f -name "*.c"}) \
$(ESP32NET_DIR)/esp32_net.c

ESP32_C_INC_DIR = -I$(ESP32NET_DIR)/lwip/src/include \
-I$(ESP32NET_DIR)/port \
-I$(ESP32NET_DIR)
ESP32_C_LIB =
ESP32_C_FLAGS = 