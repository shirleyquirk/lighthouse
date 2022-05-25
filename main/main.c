#include "main.h"
#include "wifi.h"
#include "doublereset.h"

char ssid[32] = CONFIG_STA_SSID;
char pass[32] = CONFIG_STA_PASS;
char ip[16] = CONFIG_IPV4_ADDR;
char gw[16] = CONFIG_IPV4_GW;
nvs_handle_t preferences;

static const char TAG[] = "gobo";
/*
 * Serve OTA update portal (index.html)
 */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

esp_err_t index_get_handler(httpd_req_t *req)
{
	httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}

/*
 * Handle OTA file upload
 */
esp_err_t update_post_handler(httpd_req_t *req)
{
	char buf[1000];
	esp_ota_handle_t ota_handle;
	int remaining = req->content_len;

	const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
	ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

	while (remaining > 0) {
		int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

		// Timeout Error: Just retry
		if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
			continue;

		// Serious Error: Abort OTA
		} else if (recv_len <= 0) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
			return ESP_FAIL;
		}

		// Successful Upload: Flash firmware chunk
		if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
			return ESP_FAIL;
		}

		remaining -= recv_len;
	}

	// Validate and switch to new OTA image and reboot
	if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
			return ESP_FAIL;
	}

	httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

	vTaskDelay(500 / portTICK_PERIOD_MS);
	esp_restart();

	return ESP_OK;
}

void urldecode2(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

esp_err_t wifi_manager_handler(httpd_req_t *req)
{
	char content[256]={};
	size_t recv_size = MIN(req->content_len,sizeof(content));
	int recv_len  = httpd_req_recv(req,content,recv_size);
	(void)recv_len;
	//printf("content length: %d, copied: %d\n%s\n",recv_size,recv_len,content);
	
	//parse out ssid=<url-encoded>&pass=<urlencoded>&ip=<ip>&gateway=<gateway>
	char *key = strtok(content,"=");
	char *urlenc = strtok(NULL,"&");
	urldecode2(ssid,urlenc);
	key = strtok(NULL,"=");
	urlenc = strtok(NULL,"&");
	urldecode2(pass,urlenc);
	key = strtok(NULL,"=");
	strncpy(&ip[0],strtok(NULL,"&"),16);
	key = strtok(NULL,"=");
	strncpy(&gw[0],strtok(NULL,"&"),16);
	(void)key;
	printf("ssid: %s pass: %s ip: %s gw: %s\n",ssid,pass,ip,gw); 
	nvs_set_str(preferences,"ssid",ssid);
	nvs_set_str(preferences,"pass",pass);
	nvs_set_str(preferences,"ip",ip);
	nvs_set_str(preferences,"gw",gw);
	nvs_commit(preferences);
	return ESP_OK;
}
/*
 * HTTP Server
 */
httpd_uri_t index_get = {
	.uri	  = "/",
	.method   = HTTP_GET,
	.handler  = index_get_handler,
	.user_ctx = NULL
};

httpd_uri_t update_post = {
	.uri	  = "/update",
	.method   = HTTP_POST,
	.handler  = update_post_handler,
	.user_ctx = NULL
};

httpd_uri_t wifi_manager = {
	.uri	= "/wifi",
	.method = HTTP_POST,
	.handler = wifi_manager_handler,
	.user_ctx = NULL
};

static esp_err_t http_server_init(void)
{
	static httpd_handle_t http_server = NULL;

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;

	if (httpd_start(&http_server, &config) != ESP_OK) {
		ESP_LOGE(TAG,"Failed to start http server");
		return ESP_FAIL;
	}

	httpd_register_uri_handler(http_server, &index_get);
	httpd_register_uri_handler(http_server, &update_post);
	httpd_register_uri_handler(http_server,&wifi_manager);


	return ESP_OK;
}



void app_main(void) {
	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);
	nvs_open("storage",NVS_READWRITE,&preferences);
	size_t len;
	ret = nvs_get_str(preferences,"ssid",ssid,&len);
	switch (ret)
	{
		case ESP_OK:
			printf("connecting to saved ssid %s\n",ssid);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("no saved ssid, starting softap\n");
			break;
		default:
			printf("error reading ssid from nvs\n");
	}
	ret = nvs_get_str(preferences,"pass",pass,&len);
	ret = nvs_get_str(preferences,"ip",ip,&len);
	ret = nvs_get_str(preferences,"gw",gw,&len);
	(void)len;
	//ESP_ERROR_CHECK(softap_init());
	if (double_reset()){
		ESP_ERROR_CHECK(softap_init());
	}else{
		fast_scan();
	}
	ESP_ERROR_CHECK(http_server_init());
	ledc_init();


	/* Mark current app as valid */
	const esp_partition_t *partition = esp_ota_get_running_partition();
	printf("Currently running partition: %s\r\n", partition->label);

	esp_ota_img_states_t ota_state;
	if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {
		if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
			esp_ota_mark_app_valid_cancel_rollback();
		}
	}

	while(1) vTaskDelay(10);
}
