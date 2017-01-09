//http://www.circuitmagic.com/arduino/arduino-and-bluetooth-hc-06-to-control-the-led-with-android-device/

#include <SoftwareSerial.h>

long TIMELIMIT = 9999;
int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3
 
int led = 13;

String metadata;

boolean lightBlink = false;
 
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

int counter = 0;

void setup()
{
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  Serial.println("Listening...");
  
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
  pinMode(led, OUTPUT);

}
 
void loop()
{
   
   while (bluetooth.available()) {
    
     delay(3);  
     char c = bluetooth.read();
     metadata += c; 
     
   }
   
  if (metadata.length() > 0) { Serial.println(parseData(metadata)); 
  
  if(metadata == "[RESETED]") { counter = 0; }
  
  metadata = ""; }

  delay(5);
  if(counter % 100 == 0 ) { Serial.println(counter); }
  if(counter > TIMELIMIT) { resetAndroid(); }
  counter++;
}

String parseData(String data_){

   String outputs[2];
   int starts = data_.indexOf('[') + 1;
   int ends = data_.indexOf(']');
   String split = data_.substring(starts, ends); 

   if(split.indexOf("_") == -1){ 
    if(split == "START") { return "start"; }
    else if(split == "SINGLE") { return "single shot"; }
    else if(split == "RESETED") { return "Android app was reseted after timeout"; }
    else { return "unknown command"; }
   }
   else{
   String command = split.substring(0, split.indexOf("_"));
   String value =  split.substring(split.indexOf("_") + 1);
   if(command == "CCW") { return "motor goes counter-clockwise for " + value + " steps"; } 
   else if(command == "CW") { return "motor goes clockwise for " + value + " steps"; }
   else if(command == "FLASH" && value == "ON") { return "flash is on"; }
   else if(command == "FLASH" && value == "OFF") { return "flash is off"; }
   else if(command == "PARAMETER-A") { return "settle pause is " + value; }
   else if(command == "PARAMETER-B") { return "exposure pause is " + value; }
   else if(command == "SHIFTS") { return "shots per turn " + value; }
   else { return "unknow composite command"; }
   }

}

void resetAndroid(){ bluetooth.print("RESET"); delay(250); }

