/*oskP*/
#include <cstdlib>
#include <math.h>
#include "oskp.h"
#include "common.h"


float oskIncrement=0.01;
float oskMult=2.0;

void set_oskI(float it){SAVEIT(oskIncrement,"oskp_I");}
void set_oskM(float it){SAVEIT(oskMult,"oskp_M");}

float oskP;
void oskP_task(void* parameters){
    float osk1 = (float*)(parameters);
    for(;;){
        osk1 += oskIncrement;
        float tim = log(map(sin(osk1),-1.0,1.0,0.1,10000);
        oskP = map(sin(oskMult*tim),-1,1,0,1));
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}
#define LOADIT(key,val)\
    do{\
      size_t len = sizeof(float);\
      nvs_get_blob(preferences,key,&##val,&len);\
    }while(0)

void oskp_init(){
    std::srand((micros());
    float seed = (float)rand() / (float)RAND_MAX;
    LOADIT("oskp_I",oskIncrement);
    LOADIT("oskp_M",oskMult);

    xTaskCreate(
        oskP_task,
        "oskP Task",
        4000,
        &seed,
        3,
        NULL
    );   
}