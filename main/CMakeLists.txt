idf_component_register(SRCS "oscmessages.cpp" "main.cpp" "leds.cpp" "wifi.cpp" "doublereset.cpp" "../OSC/OSCMessage.cpp"
	INCLUDE_DIRS "." ".."
	EMBED_TXTFILES web/index.html
)
