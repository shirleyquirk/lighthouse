#include "math.h"
#include "leds.h"
#include "main.h"
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
//#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
//#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_12_BIT // Set duty resolution to 13 bits
#define LEDC_MAX_DUTY               (2047) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (19531) // Frequency in Hertz. Set frequency at 5 kHz
//int led_gpios[] = {GPIO_R,GPIO_G,GPIO_B,GPIO_W};

#define DEG_TO_RAD(X) (M_PI*(X)/180)

void hsi2rgbw(float H, float S, float I) {
  uint32_t r, g, b, w;
  float cos_h, cos_1047_h;
  //H = fmod(H,360); // cycle H around to 0-360 degrees
  //H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;
  
  if(H < 2.09439) {
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    r = S*LEDC_MAX_DUTY*I/3*(1+cos_h/cos_1047_h);
    g = S*LEDC_MAX_DUTY*I/3*(1+(1-cos_h/cos_1047_h));
    b = 0;
    w = LEDC_MAX_DUTY*(1-S)*I;
  } else if(H < 4.188787) {
    H = H - 2.09439;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    g = S*LEDC_MAX_DUTY*I/3*(1+cos_h/cos_1047_h);
    b = S*LEDC_MAX_DUTY*I/3*(1+(1-cos_h/cos_1047_h));
    r = 0;
    w = LEDC_MAX_DUTY*(1-S)*I;
  } else {
    H = H - 4.188787;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    b = S*LEDC_MAX_DUTY*I/3*(1+cos_h/cos_1047_h);
    r = S*LEDC_MAX_DUTY*I/3*(1+(1-cos_h/cos_1047_h));
    g = 0;
    w = LEDC_MAX_DUTY*(1-S)*I;
  }
  
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_0,r);
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_1,g);
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_2,b);
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_3,w);   
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_0);
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_1);
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_2);
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_3);
}

void led_task(void* parameters){
    printf("Led Task Begins\n");
    float H=0.0;
    float S=1.0;
    float I=0.01;
    for(;;)
    {
        H += 0.004;
        if (H > 2*M_PI) H = H - 2*M_PI;
        hsi2rgbw(H,S,I);
        vTaskDelay(10);
    }
}

extern "C"{
void ledc_init(){
    log_printf("Initializing ledc\n");
        // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = CONFIG_GPIO_R,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    ledc_channel.channel = LEDC_CHANNEL_1;
    ledc_channel.gpio_num = CONFIG_GPIO_G;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_channel.channel = LEDC_CHANNEL_2;
    ledc_channel.gpio_num = CONFIG_GPIO_B;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    ledc_channel.channel = LEDC_CHANNEL_3;
    ledc_channel.gpio_num = CONFIG_GPIO_W;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    xTaskCreate(
        led_task,
        "LED Fade Task",
        4000,
        NULL,
        1,
        NULL
    );
  }
}


