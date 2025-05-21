// arduino json librarbar
#include <ArduinoJson.h>





// param variables
bool debug_messages = true;
int chip_outputs = 32;        // how many total outputs on your shift register(s)
int display_segments = 157;   // how many segments your display has
float brightness_steps = 20;  // max brightness integer. despite it being a float, don't make it a decimal.
float tick_speed = 0.5;      // ms between ticks
float interlace_speed = 1;    // amount of times a segment will be shown before moving on to the next segment





// do not touch variables
int registers = chip_outputs / 8;
int json_size = int(display_segments * 81.528); // calculate how big the json needs to be based on how many segments there are
DynamicJsonDocument doct(json_size);
JsonArray addresses = doct.as<JsonArray>(); // json element
bool reset_state = true;
float brightness_tm = 0;
int tick_index = 0;
int interlace_index = 0;



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

  int state_size = doct[state_int].size();

  for (int i = 0; i < state_size; ++i) {    // for all addresses in address listing

    shiftOut(data_pin, clock_pin, MSBFIRST, doct[state_int][i]);

  }
      
  digitalWrite(latch_pin, HIGH);
  digitalWrite(latch_pin, LOW);
}



void loop() {
  // put your main code here, to run repeatedly:
//Serial.print("yo");/

//  Serial.println(reset_state);/
//  Serial.print(1 == true);/

// right now the pwm speed is 1ms
// say i want the tick speed to be 0.1ms
// but keep the pwm speed at 1ms
// 1 "cycle" as im gonna call it is 1 second

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


  


  // index stuff

  
  if (tick_index % round((interlace_speed / tick_speed)) == 0) { // for interlace speed

    Serial.println("benson");
    
  }


  if (tick_index % 4 == 0) {

    Serial.println("bello ppoy banana");
    
  }



  String read_string = "";
  while (Serial.available()) {
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
        
        interlace_index = 0;
        reset_state = false;
    
        debug("teensy status");
      
      }
    }
    
  }

  tick_index += 1;
  delay(tick_speed);
  



  if (doct.memoryUsage() > json_size * 4) {
    // clear it up!
    doct.garbageCollect();
  }
  
}
