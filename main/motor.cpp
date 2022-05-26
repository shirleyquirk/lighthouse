/*motor*/
#include "motor.h"

#define motor_in1_pin CONFIG_MOTOR_POS
#define motor_in2_pin CONFIG_MOTOR_NEG
#define motor_en_pin CONFIG_MOTOR_EN
#define enc_a_pin CONFIG_ENC_A
#define enc_b_pin CONFIG_ENC_B
inline void forward(uint8_t speed){
  analogWrite(motor_en_pin,0);
  digitalWrite(motor_in1_pin,HIGH);
  digitalWrite(motor_in2_pin,LOW);
  analogWrite(motor_en_pin,speed);
}
inline void reverse(uint8_t speed){
  analogWrite(motor_en_pin,0);
  digitalWrite(motor_in1_pin,LOW);
  digitalWrite(motor_in2_pin,HIGH);
  analogWrite(motor_en_pin,speed);
}
inline void forward_brake(){
  digitalWrite(motor_in1_pin,LOW);
}
inline void reverse_brake(){
  digitalWrite(motor_in2_pin,LOW);
}

void motor_task(void* parameters){
    forward(128);
    
    for(;;){
        vTaskDelay(100);
    }
    
}
//extern "C" {
void motor_init(){
  pinMode(motor_in1_pin,OUTPUT);
  pinMode(motor_in2_pin,OUTPUT);
  pinMode(motor_en_pin,OUTPUT);
  /*ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachHalfQuad(enc_a_pin,enc_b_pin);
  encoder.setCount(0);
  timestamp_prev=micros();
  */
  xTaskCreate(
                    motor_task,          /* Task function. */
                    "Motor Task",        /* String with name of task. */
                    4000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */
  
}
//}