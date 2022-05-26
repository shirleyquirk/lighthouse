/*variables.h*/
/* all variables that are modifiable via osc */
#include <nvs_flash.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_wifi.h>
#include "leds.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"
#include "nvs.h"
#include "OSC/OSCMessage.h"
#include "Arduino.h"
#include <WiFiUdp.h>

extern "C"{
    void app_main();
}
extern char ssid[32];
extern char pass[32];
extern char ip[16];
extern char gw[16];

extern WiFiUDP udp;

//Listen to logging with nc -kluw 1 LOG_PORT
#ifdef CONFIG_LOG_DEST
  #define __log(NAME,...) do{\
    udp.beginPacket(CONFIG_LOG_DEST,CONFIG_LOG_PORT);\
    udp.NAME(__VA_ARGS__);\
    udp.endPacket();\
    }while(0)
#else
  #define __log(NAME,...) do{\
    udp.beginPacket(broadcast_ip,CONFIG_LOG_PORT);\
    udp.NAME(__VA_ARGS__);\
    udp.endPacket();\
    }while(0)
#endif/*LOG_DEST*/

#define log_printf(...) __log(printf,__VA_ARGS__)
#define log_println(...) __log(println,__VA_ARGS__)
#define log_print(...) __log(print,__VA_ARGS__)


extern nvs_handle_t preferences;
