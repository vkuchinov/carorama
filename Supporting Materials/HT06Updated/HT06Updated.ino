#include <SoftwareSerial.h>

#define STRSWITCH(STR)      char _x[16]; strcpy(_x, STR); if (false) 
#define STRCASE(STR)        } else if (strcmp(_x, STR)==0){
#define STRDEFAULT          } else {
  
SoftwareSerial BT_HC06(0, 1); // RX, TX
String metadata;

bool RUNNING = false;

void setup() {

  Serial.begin(9600);

  BT_HC06.begin(9600);
  BT_HC06.println("BT_HC06 module is ready");

}

void loop() {

while (BT_HC06.available()) {
    delay(3);  
    char c = BT_HC06.read();
    metadata += c; 
  }
  if (metadata.length() > 0) {
  Serial.println(metadata);
  char x[16];
  metadata.toCharArray(x, 16);

  STRSWITCH(x)
  {
    STRCASE ("TURN_CCW_") 
    Serial.println("turn left");
    if(!RUNNING) {  }
    STRCASE ("TURN_CW_") 
    Serial.println("turn right");
    if(!RUNNING) {  }
    STRCASE ("START") 
    if(!RUNNING) {  }
    STRCASE ("SINGLE") 
    if(!RUNNING) { }
    STRCASE ("FLASH_ON") 
    if(RUNNING) {  }
    STRCASE ("FLASH_OFF") 
    if(RUNNING) {  }
    STRCASE ("SET_PARAMETER_A_") 
    if(RUNNING) {  }
    STRCASE ("SET_PARAMETER_B_") 
    if(RUNNING) {  }
    STRCASE ("SET_SHIFTS_") 
    if(RUNNING) {  }
    STRDEFAULT 
      Serial.println("unknown command");
  }
  
  metadata = "";
}

}
