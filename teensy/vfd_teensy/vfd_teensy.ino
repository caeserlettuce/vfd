#include <SPI.h>
#include <sstream>
#include <string>
#include <vector>

// arduino json librarbar
#include <ArduinoJson.h>

int ledPin = 13;

int strobePin = 8;
int latchPin = 7;
int clockPin = 6;
int dataPin = 5;

int output_bits = 32;
  
bool hasStrobe = false;
bool debug_messages = false;
int serial_read_timeout = 0;
int display_loop_index = 0;
bool display_loop = false;
bool display_update = false;


DynamicJsonDocument doc(1024);
JsonObject obj = doc.as<JsonObject>();


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  
  pinMode(13, OUTPUT);

  // init display here
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  //pinMode(INPUT_PIN, INPUT);

  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();

  //SPI.transfer(getBit(3));
  //digitalWrite(LATCH_PIN, HIGH);
  //digitalWrite(LATCH_PIN, LOW);

  
}

void debug(String message) {
  if (debug_messages == true) {
    Serial.print("\n DBG: " + message);  
  }
}


void loop() {
  // put your main code here, to run repeatedly:

//  digitalWrite(ledPin, LOW);
//    digitalWrite(ledPin, HIGH);

      digitalWrite(ledPin, HIGH);
     
 
  serial_read_timeout = serial_read_timeout - 1;
  
  String read_string = "";
  while (Serial.available()) {
    
//      delay(3);  //delay to allow buffer to fill 
      if (Serial.available() > 0) {
        char c = Serial.read();  //gets one byte from serial buffer
        read_string += c; //makes the string readString
      }

      if (Serial.available() == 1) {

        digitalWrite(ledPin, HIGH);
        // debug(read_string);
        // json document thingy
        DynamicJsonDocument doc(1024);
        String input = read_string;
        deserializeJson(doc, input);
        obj = doc.as<JsonObject>();

        bool contains_brightness = obj.containsKey("brightness");
        bool contains_listing = obj.containsKey("listing");

//        debug(contains_brightness);
//        debug(contains_listing);
//        String brbr = obj["brightness"];
//        debug(brbr);

        if (contains_brightness == true and contains_listing == true) {

          display_loop = true;
          display_update = true;

          String brightness_tm = obj["brightness"];
          int size = obj["listing"].size();
          for (int i = 0; i < size; ++i) {    // for all addresses in address listing
              int size_two = obj["listing"][i].size();
             for (int e = 0; e < size; ++e) {    // for all addresses in address listing
               int lala = obj["listing"][i][e];
               if (lala != 0) {
                 debug("\n");
                 debug(lala);
               }
             }
         }
       }
     }
 }

  // DISPLAYING SECTION AREA TM I THINK


  if (display_loop) {
    delay(100);
    if (display_update == true) {
      debug("display update!!!");
      display_loop_index = 0;
      display_update = false;
//      delay(1000);
//      digitalWrite(ledPin, LOW);

    }


    debug("loop index: ");
    debug(display_loop_index);


    display_loop_index += 1;
  }



  


  
}


void shift(int n){
  SPI.transfer(n);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  debug("shift : ");
  debug(n);
  delay(1000);
}
