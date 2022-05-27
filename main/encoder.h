/*encoder.h*/
#ifndef ENCODER_H
#define ENCODER_H

extern int encoder_ticks_per_rev;

float encoder_theta();//-pi to pi
void encoder_init();
#endif/*ENCODER_H*/