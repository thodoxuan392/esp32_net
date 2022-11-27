ESP32_C_SOURCES_DIR = lwip port
ESP32_C_SOURCES = ${shell find  $(SOURCE_DIR) -type f -name "*.c"} \
esp32_net.c
ESP32_C_INC_DIR = -Ilibs/esp32_net/lwip/src/include \
-Ilibs/esp32_net/port
ESP32_C_LIB =
ESP32_C_FLAGS = 