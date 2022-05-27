#ifndef LEDS_H
#define LEDS_H
extern "C"{
    void ledc_init();
}
enum RGBW{
    R,G,B,W
};

void led_gamma(RGBW col,float it);
void led_max(RGBW col,float it);
#endif/*LEDS_H*/