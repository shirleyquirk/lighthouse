/*variables.h*/
/* all variables that are modifiable via osc */
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

#include "OSC/OSCMessage.h"
#include "Arduino.h"

#include "motor.h"
#include "encoder.h"
#include "wifi.h"
#include "doublereset.h"
#include "leds.h"