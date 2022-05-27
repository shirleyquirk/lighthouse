#include "ESP32Encoder.h"
#include "encoder.h"
#include "common.h"

ESP32Encoder encoder;
int32_t encoder_ticks_per_rev;
float encoder_theta(){
  return fmodf((float)encoder.getCount() / (float)encoder_ticks_per_rev + M_PI,2*M_PI)-M_PI;
}
void encoder_task(void* parameter){
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
    //log_printf("encoder: %lld",encoder.getCount());
  }
}


void encoder_init(){
  if(!nvs_get_i32(preferences,"encoder_tpr",&encoder_ticks_per_rev)){
		encoder_ticks_per_rev = 1000;
	}
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
 
