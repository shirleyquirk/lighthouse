idf_component_register(SRCS "osc.cpp" "encoder.cpp" "main.cpp" "leds.cpp" "wifi.cpp" "doublereset.cpp" "../OSC/OSCMessage.cpp" "../OSC/OSCBundle.cpp" "../OSC/OSCData.cpp" "../OSC/OSCMatch.c" "motor.cpp" "../ESP32Encoder/src/ESP32Encoder.cpp"
	INCLUDE_DIRS "." "../OSC" "../ESP32Encoder/src"
	EMBED_TXTFILES web/index.html
)
