#include "common.h"
#include "osc.h"
#include "OSCBundle.h"
#include "OSCMessage.h"
#include "encoder.h"
#include "motor.h"
#include "main.h"
#include "leds.h"
#include "oskp.h"

void osc_send_loop(void * parameters){
  //SLIPSerial.begin(115200);//230400,460800
  //IPAddress maxwell(192,168,8,5);
  OSCBundle osc_bundle;
  for(;;){
    //loop through panels
    /*
    for(int i=0;i<N_PANELS;i++){
      //loop through encoders
      panel_t *pan = &(panels[i]);
      for(int j=0;j<pan->n_encoders;j++){
        encoder_t *enc = &(pan->encoders[j]);
        if (enc->updated) {
          osc_bundle.add(enc->osc_addr).add((float)(enc->val-enc->min_val)/(float)(enc->max_val-enc->min_val));
          enc->updated=false;
        }
      }
      //loop through buttons
      for(int j=0;j<pan->n_buttons;j++){
        button_t *but = &(pan->buttons[j]);
        if (but->updated) {
          osc_bundle.add(but->osc_addr).add(but->state);
          but->updated=false;
          //special top button dealy
          if (i==3 && j==0){//top encoder button
            log_println("sending /playrandom to broadcast:5005");
            OSCMessage msg("/playrandom");
            msg.add(1);
            udp.beginPacket(broadcast_ip,5005);//we need mutexes for this shit
            msg.send(udp);
            udp.endPacket();
          }
        }
      }
    }
    if (osc_bundle.size()>0){
      udp.beginPacket(maxwell,1234);
      //SLIPSerial.beginPacket();
      osc_bundle.send(udp);
      udp.endPacket();
      //SLIPSerial.endPacket();
      osc_bundle.empty();
    }
    */
    //perfOSCWriteCounter++;
    vTaskDelay(100);
  }
}

void osc_read_loop(void *parameters){

  for(;;){
    vTaskDelay(pdMS_TO_TICKS(100));
    OSCMessage msg;
    int size;
    /*
      while(!SLIPSerial.endofPacket()) {
        if( (size =SLIPSerial.available()) > 0){
             while(size--)
                bundleIN.fill(SLIPSerial.read());
           }else{
        vTaskDelay(100);
           }
      }
      //bundle.empty();
      */
    msg.empty();
    if( (size = udp.parsePacket())>0){//asyncudp?
      if (size == 0) continue;
      while(size--)
        msg.fill(udp.read());
      if(!msg.hasError()){
        char buf[32];
        //for (int i=0;i<bundleIN.size();i++){
          //bundleIN.getOSCMessage(i)->getAddress(buf);
          msg.getAddress(buf);
          log_printf("osc msg received: %s\n",buf);
        //}
        #define MSGDISPATCH(endpoint,type,setter)\
            msg.dispatch(endpoint,[](OSCMessage &msg){if(msg.is##type(0)){setter(msg.get##type(0));}})
        #define STRDISPATCH(endpoint,buf,key)\
            msg.dispatch(endpoint,[](OSCMessage &msg){if(msg.isString(0)){msg.getString(0,buf);nvs_set_str(preferences,key,buf);nvs_commit(preferences);}})
        msg.dispatch("/encoder_ticks_per_rev",[](OSCMessage &msg){if(msg.isInt(0)){encoder_ticks_per_rev = msg.getInt(0);nvs_set_i32(preferences,"encoder_tpr",encoder_ticks_per_rev),nvs_commit(preferences);}});
        msg.dispatch("/motor/velocity",[](OSCMessage &msg){if(msg.isFloat(0)){motor_set_velocity(msg.getFloat(0));}});
        msg.dispatch("/motor/position",[](OSCMessage &msg){if(msg.isFloat(0)){motor_set_position(msg.getFloat(0));}});

        MSGDISPATCH("/motor/P",Float,motor_P);
        MSGDISPATCH("/motor/D",Float,motor_D);
        MSGDISPATCH("/motor/I",Float,motor_I);
        MSGDISPATCH("/motor/output_ramp",Float,motor_or);
        MSGDISPATCH("/motor/limit",Float,motor_limit);
        STRDISPATCH("/ssid",ssid,"ssid");
        STRDISPATCH("/password",pass,"pass");
        #define COLDISPATCH(col,parm)\
          MSGDISPATCH("/leds/" #col #parm,Float,[](float it){led_##parm(col,it);});
        COLDISPATCH(R,gamma);
        COLDISPATCH(R,max);
        COLDISPATCH(G,gamma);
        COLDISPATCH(G,max);
        COLDISPATCH(B,gamma);
        COLDISPATCH(B,max);
        COLDISPATCH(W,gamma);
        COLDISPATCH(W,max);

        //RANDAMATION
        MSGDISPATCH("/oskP/increment",Float,set_oskI);
        MSGDISPATCH("/oskP/multiplier",Float,set_oskM);
        
        MSGDISPATCH("/minDelay",Float,set_minDelay);
        MSGDISPATCH("/randDelay",Float,set_randDelay);

        msg.dispatch("/subscribe",[](OSCMessage& msg){
            if(msg.isString(0) && msg.isString(1)){
              MessageInfo in;
              msg.getString(0,&in.endpoint[0]);
              msg.getString(1,&in.destip[0]);
              tick_listeners.emplace_back(in);
            }
        });

        msg.dispatch("/motor/oskPosition",[](OSCMessage& msg){
          motor_set_oskPosition();
        });
        msg.dispatch("/leds/oskPHue",[](OSCMessage &msg)
      }else{
        log_printf("osc bundle has error:");
        //log_printf(" size: %i",bundleIN.size());
        char buf[32];
        //bundleIN.getOSCMessage(0)->getAddress(buf);
        msg.getAddress(buf);
        log_printf(" addr:%s",buf);
        log_printf(" error:%i\n",msg.getError());//bundleIN.getOSCMessage(0)->getError());
        //send it anyway?

      }
      //perfOSCReadCounter++;

    }//else udp read error
  }
}

void osc_init(){
    udp.begin(CONFIG_OSC_PORT);
    xTaskCreatePinnedToCore(
                    osc_read_loop,          /* Task function. */
                    "OSC Read Loop",        /* String with name of task. */
                    4000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL,            /* Task handle. */
                    1);                /*Core */
    
    xTaskCreate(
                    osc_send_loop,          /* Task function. */
                    "OSC Send Loop",        /* String with name of task. */
                    4000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */
}