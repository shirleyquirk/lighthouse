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

extern "C"{
    void app_main();
}
extern char ssid[32];
extern char pass[32];
extern char ip[16];
extern char gw[16];


extern nvs_handle_t preferences;