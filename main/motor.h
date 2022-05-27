/* motor.h */
#ifndef MOTOR_H
#define MOTOR_H

#include "common.h"

//extern "C"{
void motor_init();
void motor_set_position(float pos);
void motor_set_velocity(float vel);
void motor_P(float it);
void motor_I(float it);
void motor_D(float it);
void motor_or(float it);
void motor_limit(float it);

void motor_set_oskPosition();



#endif/*MOTOR_H*/