// arduino json librarbar
#include <ArduinoJson.h>





// param variables
bool debug_messages = true;
int chip_outputs = 32;        // how many total outputs on your shift register(s)
int display_segments = 157;   // how many segments your display has
float brightness_steps = 20;  // max brightness integer. despite it being a float, don't make it a decimal.
float pwm_tick_speed = 1;     // ms between pwm ticks. also don't make this a decimal.




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



void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  pinMode(13, OUTPUT);

}

void debug(String message) {
  if (debug_messages == true) {
    Serial.print("\n DBG: " + message);  
  }
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


  if (reset_state == false) {    


    // DISPLAY OFF
    digitalWrite(13, LOW);

    if ( int(interlaceRatio) != 0 && pwm_index % int(interlaceRatio) == 0 && reset_state == false) {

      // DISPLAY ON
      digitalWrite(13, HIGH);
       
    }
     
  }
  pwm_index += 1;



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
 
        doct.clear();
  
        for (int i = 0; i < size; ++i) {    // for all addresses in address listing
          int size_two = obj["listing"][i].size();
          addresses.createNestedArray();
          for (int e = 0; e < size_two; ++e) {    // for all bytes in address listing
            doct[i].add(obj["listing"][i][e]);
          }
        }
        pwm_index = 0;
        reset_state = false;
    
        debug("jaja");

        duty = 0;
        if (brightness_tm != 0) {
          duty = round ((brightness_tm / brightness_steps) * (1000 / pwm_tick_speed) );
        }

        interlaceRatio = 0;
        if(duty != 0){
          interlaceRatio = (1000 / pwm_tick_speed) / duty; // the ratio between time that the display is off vs time the display is on
        }
        
        Serial.println();
        Serial.println(duty);
        Serial.println(interlaceRatio);
      }
    }
    
  }
    
    
  delay(pwm_tick_speed);



  if (doct.memoryUsage() > json_size * 4) {
    // clear it up!
    doct.garbageCollect();
  }
  
}
