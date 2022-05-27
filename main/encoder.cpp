#include "ESP32Encoder.h"
#include "encoder.h"
#include "common.h"
const char TAG[] = "encoder";
ESP32Encoder encoder;
int32_t encoder_ticks_per_rev=2200;
float encoder_theta(){
  
  float ret = fmodf((float)encoder.getCount() / (float)encoder_ticks_per_rev + M_PI,2*M_PI)-M_PI;
  if(isnan(ret)){ESP_LOGE(TAG,"encoder_theta was NAN!\n");return 0.123456789;}
  return ret;
}
void encoder_task(void* parameter){
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
    log_printf("encoder: %lld\n",encoder.getCount());
  }
}


void encoder_init(){
  pinMode(CONFIG_ENC_A,INPUT_PULLUP);
  pinMode(CONFIG_ENC_B,INPUT_PULLUP);
  nvs_get_i32(preferences,"encoder_tpr",&encoder_ticks_per_rev);
  ESP_LOGI(TAG,"encoder_ticks_per_rev: %d",encoder_ticks_per_rev);
  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachHalfQuad(CONFIG_ENC_A,CONFIG_ENC_B);
  encoder.setCount(0);
  xTaskCreate(
                    encoder_task,          /* Task function. */
                    "Encoder Task",        /* String with name of task. */
                    4000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
}
 
