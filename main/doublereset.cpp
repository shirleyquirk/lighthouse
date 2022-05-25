/* double reset detector */
#include "main.h"
#include "doublereset.h"
#include "freertos/timers.h"

static const char TAG[] = "doublereset";
static TimerHandle_t doublereset_timer=NULL;

static void clear_doublereset(TimerHandle_t xTimer){
    nvs_set_u32(preferences,"doublereset",0);
    nvs_commit(preferences);
}
//just make sure to call after nvs_open, please and thank you.
bool double_reset(){
    uint32_t doublereset=0;
    esp_err_t ret = nvs_get_u32(preferences,"doublereset",&doublereset);
    switch (ret)
	{
		case ESP_OK:
			ESP_LOGI(TAG,"read doublereset from nvs: %d\n",doublereset);
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGI(TAG,"no doublereset found in nvs\n");
			break;
		default:
			ESP_LOGE(TAG,"error reading doublereset from nvs\n");
	}
	if (doublereset==1)
    {
		nvs_set_u32(preferences,"doublereset",0);
		nvs_commit(preferences);
	}else
    {
		nvs_set_u32(preferences,"doublereset",1);
		nvs_commit(preferences);
        doublereset_timer = xTimerCreate(
            "doublereset_timer",
            pdMS_TO_TICKS(CONFIG_DOUBLERESET_TIMEOUT),
            pdFALSE, /* one-shot, not auto-reload */
            NULL,
            clear_doublereset
        );
        if (NULL == doublereset_timer)
        {
            ESP_LOGE(TAG,"failed to create doublereset timer");
        }else
        {
            if(xTimerStart(doublereset_timer,10) != pdPASS)
            {
                ESP_LOGE(TAG,"failed to start doublereset timer");
            }
        }
	}
    ESP_LOGI(TAG,"double_reset is %s",doublereset?"true":"false");
    return (bool)doublereset;

}