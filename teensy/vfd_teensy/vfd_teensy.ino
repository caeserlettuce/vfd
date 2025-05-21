// arduino json librarbar
#include <ArduinoJson.h>





// param variables
bool debug_messages = true;
int chip_outputs = 32;        // how many total outputs on your shift register(s)
int display_segments = 157;   // how many segments your display has
float brightness_steps = 20;  // max brightness integer. despite it being a float, don't make it a decimal.
float tick_speed = 1;         // ms between ticks
float pwm_tick_speed = 1;     // ms between pwm ticks.don't make this a decimal.
float interlace_speed = 1;    // amount of times a segment will be shown before moving on to the next segment




// do not touch variables
int registers = chip_outputs / 8;
int json_size = int(display_segments * 81.528); // calculate how big the json needs to be based on how many segments there are
DynamicJsonDocument doct(json_size);
JsonArray addresses = doct.as<JsonArray>(); // json element
bool reset_state = true;
float brightness_tm = 0;
int pwm_index = 0;
float duty = 0;
float interlaceRatio = 0;
float interlace_index = 0;
int data_index = 0;
int prev_data_index = 0;
float tpc =  1000 / tick_speed;
float pwm_tpc = (tpc / tick_speed) / pwm_tick_speed;
int tick_index = 0;
float second_speed = 1000;
bool chip_power = false;
int button_state = 1;
int prev_button_state = 1;
bool vdd_relay = false;
bool vpp_relay = false;
bool inputs_set = false;

int vpp_pin = 3;
int vdd_pin = 4;
int data_pin = 5;
int clock_pin = 6;
int latch_pin = 7;
int strobe_pin = 8;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  

  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  pinMode(vpp_pin, OUTPUT); // vpp relay
  pinMode(vdd_pin, OUTPUT); // vdd relay
  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(latch_pin, OUTPUT);
  pinMode(strobe_pin, OUTPUT);

  
  digitalWrite(vpp_pin, LOW); // set vpp relay low
  digitalWrite(vdd_pin, LOW); // set vdd relay low

}

void debug(String message) {
  if (debug_messages == true) {
    Serial.print("\n DBG: " + message);  
  }
}


void send_state(int state_int) {

  // HERE is where it will retreive and send the binary to the shift register

//  Serial.println(addresses[state_int].as<String>());
//    serializeJson(addresses[state_int], Serial);

//    serializeJson(doct, Serial);
//      Serial.println(state_int);

      int state_size = doct[state_int].size();
//      Serial.print(state_size);

      for (int i = 0; i < state_size; ++i) {    // for all addresses in address listing

//        char jaja_out = addresses[state_int][i];
//        Serial.println(jaja_out);
//          serializeJson(doct[state_int][i], Serial);  

        shiftOut(data_pin, clock_pin, LSBFIRST, doct[state_int][i]);

      }


  digitalWrite(latch_pin, HIGH);
  digitalWrite(latch_pin, LOW);
}



void loop() {
  // put your main code here, to run repeatedly:
//Serial.print("yo");/
  
//
//  delay(1000);
//  digitalWrite(13, HIGH);
//  delay(1000);
//  digitalWrite(13, LOW);

//  Serial.println(reset_state);/
//  Serial.print(1 == true);/

// right now the pwm speed is 1ms
// say i want the tick speed to be 0.1ms
// but keep the pwm speed at 1ms
// 1 "cycle" as im gonna call it is 1 second

//  digitalWrite(4, HIGH);
//  delay(2000);
//  digitalWrite(4, LOW);
//  delay(2000);

  button_state = digitalRead(2);  // read power button state

  if (button_state == 1) {
    // power ON
//        Serial.println("power on");

    if (prev_button_state == 0) {
      // just switched on
        Serial.println("power just on");

      // power-up sequence
      if (vdd_relay == false) {

        Serial.println("vdd relay on");
        digitalWrite(vdd_pin, HIGH);  // vet vdd relay
        
        vdd_relay = true;
      }

      if (inputs_set == false) {
        // set inputs

        //  reset latch
        digitalWrite(data_pin, LOW);
        digitalWrite(clock_pin, HIGH);
        digitalWrite(latch_pin, LOW);
        digitalWrite(latch_pin, HIGH);
        digitalWrite(latch_pin, LOW);
        digitalWrite(strobe_pin, LOW);
        
        Serial.println("inputs set!");
        inputs_set = false;
      }

      if (vpp_relay == false) {

        digitalWrite(vpp_pin, HIGH);  // vet vpp relay

        Serial.println("vpp relay on");

        vpp_relay = true;
      }

      chip_power = true;
      
      
    }
    
  } else {
    // power OFF

    if (prev_button_state == 1) {
      Serial.println("power just off");

      chip_power = false;

      if (vdd_relay == true) {
        digitalWrite(vdd_pin, LOW);
        Serial.println("vdd relay off");
        vdd_relay = false;
      }

      if (inputs_set == false) {
        // set inputs

        //  reset latch
        digitalWrite(data_pin, LOW);
        digitalWrite(clock_pin, LOW);
        digitalWrite(latch_pin, LOW);
        digitalWrite(strobe_pin, LOW);
        
        Serial.println("inputs set!");
        inputs_set = false;
      }


      if (vpp_relay == true) {

        digitalWrite(vpp_pin, LOW);  // vet vpp relay
        
        Serial.println("vpp relay off");

        vpp_relay = false;
      }
      
    }
  

    
  }

  prev_button_state = button_state;


//  if (reset_state == false && chip_power == true) {
//
//    // DISPLAY OFF
//    digitalWrite(13, LOW);
//
//    if ( int(interlaceRatio) != 0 && pwm_index % int(interlaceRatio) == 0 && reset_state == false) {
//
//      // DISPLAY ON
//      digitalWrite(13, HIGH);
//
//      interlace_index += 1;
//      
//      
//      if ( round(interlace_index) % round(interlace_speed) == 0 && reset_state == false) {
//        // jaja data time
//
////        Serial.print(data_in/dex);
//        
//        data_index += 1;
//        if ( data_index >= doct.size()) {
//          data_index = 0;
//        }
//      }
//    }
//
//
//     
//  }

  if (reset_state == false && chip_power == true) {

     if ( round(interlace_index) % round(interlace_speed) == 0 && reset_state == false) {
        // jaja data time

//        Serial.print(data_in/dex);
        
        data_index += 1;
        if ( data_index >= doct.size()) {
          data_index = 0;
        }
      }
    
  }

  


                   // dont ask why its divided by tick speed twice because i do not know either  
  if (tick_index % round( (pwm_tick_speed / tick_speed) / tick_speed) == 0) {   // PWM INDEX
    pwm_index += 1;
  }
  
  if (tick_index % round( (interlace_speed / tick_speed) / tick_speed) == 0) {  // INTERLACE INDEX
    interlace_index += 1;

    if (data_index != prev_data_index) {
      prev_data_index = data_index;
      send_state(round(data_index));
    }
  }




//if (reset_state == false && chip_power == true) {
//
//   
//   if (data_index != prev_data_index) {
////    Serial.println("hi");
//    prev_data_index = data_index;
//    send_state(round(data_index));
//  }
//
//  
//  data_index += 1;
//  if ( data_index >= doct.size()) {
//    data_index = 0;
//  }
//}
  



  String read_string = "";
  while (Serial.available()) {
//    Serial.print("poo");/
    
//    delay(3);  //de/lay to allow buffer to fill /
    if (Serial.available() > 0) {
      char c = Serial.read();  //gets one byte from serial buffer
      read_string += c; //makes the string readString
    }

    if (Serial.available() == 1) {
      DynamicJsonDocument doc(json_size);
      String input = read_string;
      deserializeJson(doc, input);
      JsonObject obj = doc.as<JsonObject>(); // json element
       
      bool contains_brightness = obj.containsKey("brightness");
      bool contains_listing = obj.containsKey("listing");

      if (contains_brightness == true and contains_listing == true) {
        brightness_tm = obj["brightness"].as<float>();
//        Serial.print(obj["brightness/"].as<float>());
            
        int size = obj["listing"].size();

        // INPUT CONFIRMED, RUN ONCE STUFF HAPPENS HERE!
// 
        doct.clear();
  
        for (int i = 0; i < size; ++i) {    // for all addresses in address listing
          int size_two = obj["listing"][i].size();
          addresses.createNestedArray();
          for (int e = 0; e < size_two; ++e) {    // for all bytes in address listing
            doct[i].add(obj["listing"][i][e]);
          }
        }

//        doct.clear();
//        addresses.createNestedArray();
//        doct[0].add(B00000000);
//        doct[0].add(B00000000);
//        doct[0].add(B11000000);
//        doct[0].add(B00000000);

        
//        shiftOut(data_pin, clock_pin, LSBFIRST, B00000000);
//        shiftOut(data_pin, clock_pin, LSBFIRST, B11000000);
//        shiftOut(data_pin, clock_pin, LSBFIRST, B00000000);
//        shiftOut(data_pin, clock_pin, LSBFIRST, B00000000);
//        
//        digitalWrite(latch_pin, HIGH);
//        digitalWrite(latch_pin, LOW);


        
//        pwm_index = 0;
        interlace_index = 0;
        data_index = 0;
        reset_state = false;
    
        debug("teensy status");

        duty = 0;
        if (brightness_tm != 0) {
          duty = round ((brightness_tm / brightness_steps) * pwm_tpc );
        }

        interlaceRatio = 0;
        if(duty != 0){
          interlaceRatio = pwm_tpc / duty; // the ratio between time that the display is off vs time the display is on
        }

        

        
//        Serial.println();
//        Serial.println(duty);
//        Serial.println(interlaceRatio);
      }
    }
    
  }
    
  tick_index += 1;
//  if (tick_index >= 10000 / tick_speed) {
//    tick_index = 0;
//    Serial.println("tick index reset!");
//  }
  delay(tick_speed);
  



  if (doct.memoryUsage() > json_size * 4) {
    // clear it up!
    doct.garbageCollect();
  }
  
}
