#!/usr/bin/bash
curl --progress-bar -X POST --data-binary @build/esp32-softap-ota.bin http://192.168.4.1/update  | tee /dev/null
