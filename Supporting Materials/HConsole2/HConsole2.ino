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

int received = 0;

int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3


SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

class Listener {
  private:

    int port;

  public:

    bool state = true;
    bool handshaked = false;
    int last;

    Listener(int port_);
    void set(bool state_);
    void sendIdle();
    void sendIdle(int times_);
    void getOK();
    void getStop();
};


Listener::Listener(int port_) {
  this->port = port_;
  this->last = millis();
}
void Listener::set(bool state_) {

  delay(750);
  
  this->state = state_;
  if (this->state) {
    digitalWrite(this->port, HIGH);
  } else {
    digitalWrite(this->port, LOW);
  }

  delay(750);
  
}
void Listener::sendIdle() {

  delay(750);
  
  while (!handshaked) {
    bluetooth.print("![IDLE]");
    this->getOK();
  } this->set(true);
  handshaked = false;

  delay(750);
  
}

void Listener::sendIdle(int times_) { bluetooth.print("![IDLE]"); }

void Listener::getOK() {

  String metadata = "";

  while (bluetooth.available()) {

    delay(3);
    char c = bluetooth.read();
    metadata += c;

  }

  if (metadata.length() > 0 && metadata.indexOf("OK") >= 0) {
    String m = "[HANDSHAKE] in " + String((millis() - this->last)) + " millis";
    this->handshaked = true;
    this->last = millis();
    Serial.println(m);
    delay(50);
  }

}

void Listener::getStop() {

  String metadata = "";

  while (bluetooth.available()) {

    delay(3);
    char c = bluetooth.read();
    metadata += c;

  }

  if (metadata.length() > 0 && metadata.indexOf("STOP") >= 0) {
    this->handshaked = true;
  }

}

long starts; 
long ends;

int counter = 0;
Listener *listener;

void setup()

{

  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  while (!Serial) { // wait for serial port 
  }
  Serial.println("WELCOME TO HC-06 TEST");

  listener = new Listener(11);
  listener->set(true);
  
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
  //bluetooth.write("AT+NAMECARORAMA");

  Serial.println("LISTENING...");
  bluetooth.print("[[![IDLE]]]");
  
}

void loop()
{
//
//  String metadata = "";
//
//  if(listener){
//  while(bluetooth.available())  { delay(3); char c = bluetooth.read(); metadata += c; }
//
//  if(checkPacket(metadata)) 
//  
//  { 
//    
//    if(getCommand(metadata).indexOf("OK") < 0) 
//  
//    { 
//
//    delay(500);
//    received++;
//    int val = metadata.substring(metadata.indexOf("_") + 1, metadata.indexOf("]")).toInt();
//    String m = getCommand(metadata) + " from " + received + " or " + round(val/received*100);
//    String b = "[[[[[<![" + m + "]>]]]]]";
//    
//    bluetooth.print(b); 
//    //Serial.println(m); 
//    delay(500);
//    
//    } 

String b = "![A" + String(counter) + "]";
bluetooth.print(b); 
delay(200);
counter++;
    
  } 
    
    
    //else { if(metadata.length() > 1) { Serial.println("invalid packet"); delay(50); } }
  
  //if (getCommand(metadata).indexOf("STR") >=0) { listener = false; delay(5000); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("SINGLE") >=0) { Serial.println("[SINGLE]"); listener = false; digitalWrite(listenerPin, LOW); delay(2500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("FLASH_ON") >=0) { Serial.println("[FLASH_ON]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("FLASH_OFF") >=0) { Serial.println("[FLASH_OFF]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("CW") >=0 && metadata.indexOf("CCW") < 0) { Serial.println("[CW]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("CCW") >=0) { Serial.println("[CCW]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("STEPS") >=0) { Serial.println("[STEPS]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("STABILIZATION") >=0) { Serial.println("[STABILIZATION]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  if (metadata.indexOf("EXPOSURE") >=0) { Serial.println("[EXPOSURE]"); listener = false; digitalWrite(listenerPin, LOW); delay(1500); while(!listener) { resetAndroid(); }}
  //  }

  //Serial.println(metadata);
  //}
  //sendIdle();
  

//}

String getCommand(String data_){

    return data_.substring(data_.indexOf("["), data_.indexOf("]"));
}

bool checkPacket(String data_){

    //<![(length)command]> without handshaking
    //<?[(length)command]> with handshaking

    //!/?[ should have lower index than ]
    //and command length should be equal to length

    int starts, ends;
    
    if(data_.indexOf('!') >= 0) { starts = data_.indexOf("!["); }
    else if (data_.indexOf('?') >=0 ) { starts = data_.indexOf("!["); }
    else { return false; }
    
    if(data_.indexOf("]") >= 0) { ends = data_.indexOf("!["); }
    else { return false; }

    if(starts > ends) { return false; }

    //extracting length and message
    int len = data_.substring(data_.indexOf("(") + 1, data_.indexOf(")")).toInt();
    String command =  data_.substring(data_.indexOf(")") + 1, data_.indexOf("]"));
    if(command.length() == len) { return true; }

    return false;

}

void sendIdle(){   bluetooth.print("![IDLE]"); }

void resetAndroid(){

  String metadata = "";

  starts = millis();
  //Serial.println("RESETING COUNTER");
  sendIdle();
  
  while(bluetooth.available())  // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    delay(3);  
    char c = bluetooth.read();
    metadata += c; 
    //Serial.print((char)bluetooth.read());  
  }

  //if (getCommand(metadata).indexOf("OK") >=0) { delay(5); listener = true; digitalWrite(listenerPin, HIGH); delay(5); Serial.println("[HANDSHAKED]"); Serial.println(millis() - starts); starts = millis(); }
  
}


