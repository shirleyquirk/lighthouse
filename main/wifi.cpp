#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "main.h"
#include "freertos/timers.h"
#include "wifi.h"

static const char* TAG = "wifi";

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();//xTimerStart(tmr,10);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}
/*
 * WiFi configuration
 */
extern "C"
{
esp_err_t softap_init(void)
{
	esp_err_t res = ESP_OK;

	res |= esp_netif_init();
	res |= esp_event_loop_create_default();
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	res |= esp_wifi_init(&cfg);

	wifi_config_t ap_config = {
		.ap = {
			{.ssid = CONFIG_WIFI_SSID},
			.ssid_len = strlen(CONFIG_WIFI_SSID),
			//.channel = 6,
			.max_connection = 3
		}
	};
    if (0==strlen(CONFIG_WIFI_PASS)) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }else{
        strcpy((char*)ap_config.ap.password,&CONFIG_WIFI_PASS[0]);
        ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

	res |= esp_wifi_set_mode(WIFI_MODE_AP);
	res |= esp_wifi_set_config(WIFI_IF_AP, &ap_config);
	res |= esp_wifi_start();

	return res;
}

/* Initialize Wi-Fi as sta and set scan method */
void fast_scan()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // Initialize default station as network interface instance (esp-netif)
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);


        // Initialize and start WiFi
    wifi_config_t sta_config = {
        .sta = {
            //.ssid = ssid,
            //.password = pass,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold = {.rssi = -127,
                .authmode = WIFI_AUTH_OPEN},
        }
    };
    

    strcpy((char*)sta_config.sta.ssid,&ssid[0]);
    strcpy((char*)sta_config.sta.password,&pass[0]);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_LOGI(TAG,"starting station mode with ssid %s and password %s",ssid,pass);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
}/*extern c*/