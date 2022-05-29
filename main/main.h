/*variables.h*/
/* all variables that are modifiable via osc */
#ifndef MAIN_H
#define MAIN_H

#include <nvs_flash.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <sys/param.h>
#include <esp_wifi.h>
#include "string.h"

#include "Arduino.h"

#include "motor.h"
#include "encoder.h"
#include "wifi.h"
#include "doublereset.h"
#include "leds.h"
#include "common.h"
#include "motor.h"
#include "osc.h"
#include "oskp.h"
#include <vector>

extern char ssid[32];
extern char pass[32];

extern float minDelay;
extern float randDelay;

void set_randDelay(float it);
void set_minDelay(float it);


struct MessageInfo{
    float probability;
    char endpoint[64];
    char destip[16];
};

extern std::vector<MessageInfo> tick_listeners;//todo: nvs!

#endif/*MAIN_H*/
