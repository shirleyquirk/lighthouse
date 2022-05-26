/* common.h */
#ifndef COMMON_H
#define COMMON_H

#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFiUdp.h>

extern char ssid[32];
extern char pass[32];
extern char ip[16];
extern char gw[16];


extern nvs_handle_t preferences;

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
//#define log_println(...) __log(println,__VA_ARGS__)
//#define log_print(...) __log(print,__VA_ARGS__)

#endif/*COMMON_H*/