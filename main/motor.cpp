/*motor*/
#include "motor.h"
#include "encoder.h"
#include "driver/ledc.h"
#include "math.h" //M_PI,fmodf
const char TAG[]="motor";
#ifdef CONFIG_MOTOR_TYPE_L298N
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_MAX_DUTY (4095)
#define motor_in1_pin CONFIG_MOTOR_POS
#define motor_in2_pin CONFIG_MOTOR_NEG
#define motor_en_pin CONFIG_MOTOR_EN
#define enc_a_pin CONFIG_ENC_A
#define enc_b_pin CONFIG_ENC_B
void forward(float speed){
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_4,0);
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_4);
  digitalWrite(motor_in1_pin,HIGH);
  digitalWrite(motor_in2_pin,LOW);
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_4,(uint32_t)(speed*LEDC_MAX_DUTY));
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_4);
}
void reverse(float speed){
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_4,0);
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_4);
  digitalWrite(motor_in1_pin,LOW);
  digitalWrite(motor_in2_pin,HIGH);
  ledc_set_duty(LEDC_MODE,LEDC_CHANNEL_4,(uint32_t)(speed*LEDC_MAX_DUTY));
  ledc_update_duty(LEDC_MODE,LEDC_CHANNEL_4);
}
inline void forward_brake(){
  digitalWrite(motor_in1_pin,LOW);
}
inline void reverse_brake(){
  digitalWrite(motor_in2_pin,LOW);
}
#elif CONFIG_MOTOR_TYPE_BTS7960
inline void forward(uint8_t speed){
    analogWrite(CONFIG_MOTOR_PWM_R,0);
    analogWrite(CONFIG_MOTOR_PWM_F,speed);
}
inline void reverse(uint8_t speed){
    analogWrite(CONFIG_MOTOR_PWM_F,0);
    analogWrite(CONFIG_MOTOR_PWM_R,speed);
}
#endif
enum MotorMode{
    Velocity,Position
};
enum MotorDir{
    Forward,Reverse,Stop
};

MotorMode mode;
MotorDir dir;
float goal_position=0.0;
unsigned long timestamp_prev;
#ifdef CONFIG_MOTOR_TYPE_BTS7960
float limit=0.2;
#else
float limit=1.0;
#endif
float output_prev = 0;
float error_prev = 0;
float integral_prev = 0;

//Configurables
float P=2.0; 
float I=4.0;
float D=1.0;
float output_ramp = 10.0; // i dunno. 1.0? full reverse to full forward in 2s?



void motor_P(float it ){ SAVEIT(P,"pid_prop");}
void motor_I(float it ){ SAVEIT(I,"pid_int"); }
void motor_D(float it ){ SAVEIT(D,"pid_int");}
void motor_or(float it){SAVEIT(output_ramp,"pid_or");}
void motor_limit(float it){SAVEIT(limit,"motor_limit");}

void motor_velocity(float vel){
    //vel = constrain(vel,-1.0,1.0);
    if (vel > 0.0){
        forward(vel);
        dir = Forward;
    }else if(vel < 0.0){
        reverse(-vel);
        dir = Reverse;
    }else{
        forward(0);
        dir = Stop;
    }
}

inline float circlify(float x){
    return fmodf(x+M_PI,2*M_PI)-M_PI;
}
/*osc commands*/
void motor_set_velocity(float vel){
    mode = Velocity;
    vel = constrain(vel,-limit,limit);
    output_prev = vel;
    motor_velocity(vel);
}
void motor_set_position(float pos){
    mode = Position;
    goal_position = circlify(pos); //values from -pi to pi, wrap around
}

void motor_set_oskPosition(){
    mode = Position;
    goal_position = map(oskP,0,1,-M_PI,M_PI);
}


float pidoperator(float error){
    if(isnan(error)){ESP_LOGE(TAG,"error was NAN!\n");return 0.0;}
    static int log_count=0;

    // calculate the time from the last call
    unsigned long timestamp_now = micros();
    float Ts = (timestamp_now - timestamp_prev) * 1e-6;
    // quick fix for strange cases (micros overflow)
    if(Ts <= 0 || Ts > 0.5) Ts = 1e-3; 
    if(isnan(Ts)){ESP_LOGE(TAG,"Ts was NAN! timestamp_now: %lu timestamp_prev: %lu",timestamp_now,timestamp_prev);Ts=0.0;}

    // u(s) = (P + I/s + Ds)e(s)
    // Discrete implementations
    // proportional part 
    // u_p  = P *e(k)
    float proportional = P * error;
    // Tustin transform of the integral part
    // u_ik = u_ik_1  + I*Ts/2*(ek + ek_1)
    float integral = integral_prev + I*Ts*0.5*(error + error_prev);
    if(isnan(integral)){ESP_LOGE(TAG,"integral was NAN! integral_prev: %f I: %f error_prev:%f",integral_prev,I,error_prev); integral=0.0;}
    // antiwindup - limit the output voltage_q
    integral = constrain(integral, -limit, limit);
    // Discrete derivation
    // u_dk = D(ek - ek_1)/Ts
    float derivative = D*(error - error_prev)/Ts;

    // sum all the components
    float output = proportional + integral + derivative;
    // antiwindup - limit the output variable
    output = constrain(output, -limit, limit);

    // limit the acceleration by ramping the output
    float output_rate = (output - output_prev)/Ts;
    if (output_rate > output_ramp)
        output = output_prev + output_ramp*Ts;
    else if (output_rate < -output_ramp)
        output = output_prev - output_ramp*Ts;

    // saving for the next pass
    integral_prev = integral;
    output_prev = output;
    error_prev = error;
    timestamp_prev = timestamp_now;
    if (!(++log_count % 500)){
        log_printf("pidcontroller: error: %f, prop: %f, int: %f, der: %f, output: %f,error_prev: %f, Ts: %f, limit: %f\n",error,proportional,integral,derivative,output,error_prev,Ts,limit);
    }
    
    return output;
}
void motor_task(void* parameters){
    ESP_LOGE(TAG,"Started Motor Task\n");
    //get configurables
    size_t len = sizeof(float);
    nvs_get_blob(preferences,"pid_prop",&P,&len);
    len = sizeof(float);
    nvs_get_blob(preferences,"pid_int",&I,&len);
    len = sizeof(float);
    nvs_get_blob(preferences,"pid_der",&D,&len);
    len = sizeof(float);
    nvs_get_blob(preferences,"pid_or",&output_ramp,&len);
    len = sizeof(float);
    nvs_get_blob(preferences,"motor_limit",&limit,&len);
    
    motor_set_velocity(-1.0);
    vTaskDelay(pdMS_TO_TICKS(200));
    motor_set_velocity(-0.2);
    timestamp_prev=micros();
    for(;;){
        float error = circlify(goal_position-encoder_theta());
        float vel = pidoperator(error);
        switch (mode){
        case Velocity: break;
        case Position:
            motor_velocity(vel);
        }
        vTaskDelay(pdMS_TO_TICKS(10));

    }
    
}
//extern "C" {
void motor_init(){
#ifdef CONFIG_MOTOR_TYPE_L298N
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_MODE,
      .duty_resolution = LEDC_TIMER_12_BIT,
      .timer_num = LEDC_TIMER_1,
      .freq_hz = 19531,
      .clk_cfg = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
  ledc_channel_config_t ledc_channel = {
      .gpio_num = CONFIG_MOTOR_EN,
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL_4,
      .intr_type = LEDC_INTR_DISABLE,
      .duty = 0,
      .hpoint = 0
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  pinMode(motor_in1_pin,OUTPUT);
  pinMode(motor_in2_pin,OUTPUT);
  //pinMode(motor_en_pin,OUTPUT);
#elif CONFIG_MOTOR_TYPE_BTS7960
  //pinMode(CONFIG_MOTOR_EN_R,OUTPUT);
  //pinMode(CONFIG_MOTOR_EN_F,OUTPUT);
  pinMode(CONFIG_MOTOR_PWM_F,OUTPUT);
  pinMode(CONFIG_MOTOR_PWM_R,OUTPUT);
  pinMode(CONFIG_MOTOR_IS_F,INPUT);
  pinMode(CONFIG_MOTOR_IS_R,INPUT);
  //digitalWrite(CONFIG_MOTOR_EN_R,HIGH);
  //digitalWrite(CONFIG_MOTOR_EN_F,HIGH);
#endif

 
  xTaskCreate(
                    motor_task,          /* Task function. */
                    "Motor Task",        /* String with name of task. */
                    4000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */
  
}
//}