idf_component_register(SRCS "main.c" "leds.c" "wifi.c" "doublereset.c"
	INCLUDE_DIRS "."
	EMBED_TXTFILES web/index.html
)
