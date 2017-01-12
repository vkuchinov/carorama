/*
 * 
 Example Bluetooth Serial Passthrough Sketch
 by: Jim Lindblom
 SparkFun Electronics
 date: February 26, 2013
 license: Public domain

 This example sketch converts an RN-42 bluetooth module to
 communicate at 9600 bps (from 115200), and passes any serial
 data between Serial Monitor and bluetooth module.
 
 */
 
#include <SoftwareSerial.h>  

int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3

int listenerPin = 11;
bool listener = true;

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

long starts; 
long ends;

int counter = 0;
  
void setup()

{

  pinMode(listenerPin, OUTPUT);
  digitalWrite(listenerPin, HIGH);
  
  Serial.begin(9600);  // Begin the serial monitor at 9600bps

  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600

  Serial.println("RESETING COUNTER");
  bluetooth.print("![IDLE]");
  
}

void loop()
{

  String metadata = "";

  if(listener){
  while(bluetooth.available())  // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    delay(3);  
    char c = bluetooth.read();
    metadata += c; 
    //Serial.print((char)bluetooth.read());  
  }

  //if (metadata == "[OK]") {Serial.println("HANDSHAKED"); listener = true; digitalWrite(listenerPin, HIGH);  }
  //if (metadata.length() > 0) { Serial.println(metadata); }

  if (metadata.indexOf("START") >=0) { Serial.println("[START]"); listener = false; digitalWrite(listenerPin, LOW); delay(5000); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("SINGLE") >=0) { Serial.println("[SINGLE]"); listener = false; digitalWrite(listenerPin, LOW); delay(2500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("FLASH_ON") >=0) { Serial.println("[FLASH_ON]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("FLASH_OFF") >=0) { Serial.println("[FLASH_OFF]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("CW") >=0 && metadata.indexOf("CCW") < 0) { Serial.println("[CW]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("CCW") >=0) { Serial.println("[CCW]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("STEPS") >=0) { Serial.println("[STEPS]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("STABILIZATION") >=0) { Serial.println("[STABILIZATION]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  if (metadata.indexOf("EXPOSURE") >=0) { Serial.println("[EXPOSURE]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  }

}

void resetAndroid(){

  String metadata = "";

  starts = millis();
  //Serial.println("RESETING COUNTER");
  bluetooth.print("![IDLE]");
      
  while(bluetooth.available())  // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    delay(3);  
    char c = bluetooth.read();
    metadata += c; 
    //Serial.print((char)bluetooth.read());  
  }

  if (metadata == "[OK]") { delay(5); listener = true; digitalWrite(listenerPin, HIGH); delay(5); Serial.println("[HANDSHAKED]"); Serial.println(millis() - starts); starts = millis(); }
  
}


