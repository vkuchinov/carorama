/*
 *  УПРАВЛЕНИЕ КАМЕРОЙ ПО ОДНОЙ ОСЯМ v.0.9β
 * 
 *                                      ПОРТ            ПИТАНИЕ      DEBOUNCING [ДРЕБЕЗЖАНИЕ]     ШАГ               
 *                                        
 *  1 x Bluetooth HC-06 модуль          D1, D2, GND     5В           -
 *  1 х пьезодатчик RobotDyn            D12             5В           -
 *  1 x реле DPDT                       D5,             5В           -
 *  1 x реле OMIRON для вспышки         D6                           -
 *  1 x светодиод на 220Ω резисторе     D13             5В           -
 *  1 x NEMA 1X двигатель на SD8825     D8, D9, D7      5В/12В       -                            1/32
 *  
 *  1 x RESET кнопка                    RST             5В           -
 *  
 *  REFERENCES:
 *  http://playground.arduino.cc/Code/Bounce [debounce library]
 *  http://ai2.appinventor.mit.edu/
 *  https://github.com/carlosmaniero/Arduino-Async-Delay/blob/master/examples/01_hello.ino
 *  http://www.makeuseof.com/tag/arduino-delay-function-shouldnt-use/
 *  
 *  http://www.martyncurrey.com/arduino-with-hc-05-bluetooth-module-at-mode/
 *  https://learn.sparkfun.com/tutorials/using-the-bluesmirf
 *  
 *  LATEST ANDROID BUILD
 *  http://ai2.appinventor.mit.edu/?locale=en#6339780298932224
 *  
 *  @author   Vladimir V. KUCHINOV
 *  @email    helloworld@vkuchinov.co.uk
 *  
 */

/*
 *     DRV8825 BOARD SCHEME
 *     
 *            B R K G
 *            | | | |
 *     *********************
 *     *            *****  *   G: GREEN
 *     *            *   *  *   R: RED
 *     *   DRV8825  *   *  *   B: BLUE
 *     *            *   *  *   K: BLACK
 *     *            *****  *
 *     *********************
 * 
 */

 /*
  *  LISTENER
  *  
  *  1. NOT LISTENING WHILE DOING SOMETHING
  * 
  * 
  */

  
#include<SoftwareSerial.h>

SoftwareSerial BT_HC06(2, 3); // TX, RX
String metadata;

//VARIALBES

int  TIMELIMIT = 9999; //временнной лимит для перезагрузки Android приложения
int  counter = 0;

int  MOTOR_RESOLUTION = 400;           // физическое количество шагов на круг 
int  GEARBOX = 1;                      // шестерная передача
long NUM_OF_IMPULSES_PER_TURN = 32000; // динамический параметр, пересчитывается при
                                       // инициализации мотора

     //высчитывается по формуле шаги [200 или 400] * микрошаги [32] * шестерная передача
                                      
int  SHIFTS = 8;        // количество снимков на круг [управляется с телефона]
int  INTERVAL_A = 3000; // задержка для стабилизации [управляется с телефона]
int  INTERVAL_B = 2500; // выдержка [управляется с телефона]

bool  RUNNING = false;
bool  LISTENING = true;

#define STRSWITCH(STR)      char _x[16]; strcpy(_x, STR); if (false) 
#define STRCASE(STR)        } else if (strcmp(_x, STR)==0){
#define STRDEFAULT          } else {

/*******************************
 * BUZZ CLASS: ПЬЕЗОДИНАМИК 
 ******************************/

#define C 0
#define D 1
#define E 2
#define F 3
#define G 4
#define A 5
#define B 6

class Buzz {
private:
 int port;
 
public:
 Buzz(int port_);
 void inits();
 void play(int note_, int length_);
};

Buzz::Buzz(int port_) { this->port = port_; }
void Buzz::inits() { pinMode(this->port, OUTPUT); delay(50); Serial.println("Sound is initialized."); }
void Buzz::play(int note_, int length_) {  


     //C, D, E, F, G, A, B
     int notes[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014 };
     int timeHigh = notes[note_]; 

     for(int i = 0; i < length_; i++){
         digitalWrite(this->port, HIGH);
         delayMicroseconds(timeHigh);
         digitalWrite(this->port, LOW);
         delayMicroseconds(timeHigh);
     }
}

/*******************************
 * LED CLASS: СВЕТОДИОД
 ******************************/
 
class LED {
private:

 int port;
 
public:
 LED(int port_);
 void inits();
 void turnOn();
 void turnOff();
};

LED::LED(int port_) { this->port = port_; }
void LED::inits() { pinMode(this->port, OUTPUT); delay(50); Serial.println("LED is initialized."); }
void LED::turnOn() { digitalWrite(this->port, HIGH); }
void LED::turnOff() { digitalWrite(this->port, LOW); }

/***********************************
 * RELAY CLASS: Реле одиночное
 **********************************/

class Relay {
private:
 int port;
 
public:
 Relay(int port_);
 void inits();
 void turnOn();
 void turnOff();
 bool enabled = true;
};

Relay::Relay(int port_) { this->port = port_; }
void Relay::inits() { pinMode(this->port, OUTPUT); delay(250); Serial.println("Relay is initialized."); }
void Relay::turnOn() { if(this->enabled) { digitalWrite(this->port, HIGH); } }
void Relay::turnOff() { digitalWrite(this->port, LOW); } 

/*************************************************************************
 * TIMER CLASS: USE IT INSTEAD OF 'BLOCKING' DELAY() / DELAYMICROSECONDS()
 ************************************************************************/

class Listener {
private:
 bool state = true;
 bool last = false;
 
public:
 Listener();
 void set(bool state_);
 bool get();
};

Listener::Listener() {  }
void Listener::set(bool state_) { this->last = this->state; this->state = state_; if(this->state) { Serial.println("LISTENING"); } else { Serial.println("STOP LISTENING"); } }
bool Listener::get() {  return this->state; }

Listener *listener;

/*******************************************************************
 * STEPPER MOTOR CLASS: ШАГОВЫЙ ДВИГАТЕЛЬ ЧЕРЕЗ SD8825 
 ******************************************************************/

#define CW   1
#define CCW -1

class StepperMotor {
private:

 int ports[3]; //0: dir, 1: pull, 2: enable
 int dir = 1;
 int pause = 3;
 int micropause = 2;
 int resolution; //1: 1/1, 2: 1/2, 4: 1/4, 8: 1/8, 16: 1/16...
 int fullTurn = MOTOR_RESOLUTION; //200 by default, 400 high resolution motorp

public:
 StepperMotor(int dir_, int pull_, int enable_, int resolution_);
 void inits();
 void stepCW();   //один шаг по часовой стрелки
 void stepCCW();  //один шаг против часовой стрелки
 void moveNumOfSteps(int steps_, int dir_);
 void shift(int dir_);  
};

StepperMotor::StepperMotor(int dir_, int pull_, int enable_, int resolution_) { this->ports[0] = dir_; this->ports[1] = pull_; this->ports[2] = enable_; resolution = resolution_; }
void StepperMotor::inits() { 
  
    pinMode(this->ports[0], OUTPUT);  
    pinMode(this->ports[1], OUTPUT); 
    pinMode(this->ports[2], OUTPUT); 
    digitalWrite(this->ports[2], HIGH), 
    delay(50); 
    Serial.println("Stepper is initialized."); 
    NUM_OF_IMPULSES_PER_TURN = fullTurn * resolution * GEARBOX;
    Serial.print("There are ");
    Serial.print(NUM_OF_IMPULSES_PER_TURN);
    Serial.print(" impulses per full turn.\n");
    
    }
    
void StepperMotor::stepCW(){

    digitalWrite(this->ports[0], true); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(micropause);
    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(micropause);
 
    digitalWrite(this->ports[0], false); 
    delayMicroseconds(pause);
    
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(micropause);
    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(micropause);
  
}

void StepperMotor::stepCCW(){

    digitalWrite(this->ports[0], true); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(micropause);
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(micropause);
 
    digitalWrite(this->ports[0], false); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(micropause);
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(micropause);
  
}

void StepperMotor::moveNumOfSteps(int steps_, int dir_){ listener->set(false); digitalWrite(ports[2], LOW); for(int s = 0; s < steps_; s++){ if(dir_ == 1) { stepCW(); } else { stepCCW(); }} digitalWrite(this->ports[1], LOW); digitalWrite(ports[2], HIGH); listener->set(true); }
void StepperMotor::shift(int dir_){ digitalWrite(ports[2], LOW); long steps = floor(NUM_OF_IMPULSES_PER_TURN / SHIFTS); for(int s = 0; s < steps; s++){ if(dir_ == 1) { stepCW(); } else { stepCCW(); }} digitalWrite(this->ports[1], LOW); digitalWrite(ports[2], HIGH); }



LED *led;
Buzz *sound;
Relay *flash;
Relay *shutter;

StepperMotor *motorA;


void setup() {

  listener->set(false);
  
  Serial.begin(9600);

  BT_HC06.begin(115200);
  BT_HC06.print("$");  
  BT_HC06.print("$");
  BT_HC06.print("$");  
  delay(100);
  BT_HC06.println("U,9600,N"); 
  BT_HC06.begin(9600);
  
  //LED
  led = new LED(13);
  led->inits();
  
  //sound
  sound = new Buzz(12);
  sound->inits();

  //relays
  shutter = new Relay(5);
  shutter->inits();

  //flash
  //flash = new Relay(6);
  //flash->inits();
  
  //steppers
  motorA = new StepperMotor(9, 8, 7, 32);
  motorA->inits();

  delay(500);
  for(int i = 0; i < 4; i++){
  motorA->shift(CW); //test
  }
  delay(1500);
  for(int i = 0; i < 4; i++){
  motorA->shift(CCW); //test
  }
  delay(1500);
  shutter->turnOn();
  led->turnOn();
  delay(1500);
  shutter->turnOff();
  led->turnOff();

  listener->set(true);
  
} 

void loop() { listenBluetooth(); }

void scenarioA(){

     listener->set(false);
     
     Serial.println("Main Sequence");
     RUNNING = true;

     led->turnOff();
     sound->play(A, 2000);
     delay(10000);

     for(int i = 0; i < SHIFTS; i++){

           shutter->turnOn(); 
           delay(INTERVAL_B);
           shutter->turnOff();
           motorA->shift(CW);
           delay(INTERVAL_A);
     }

     delay(1500);

     endSignal();
     listener->set(true);

}

void singleShot(){

    listener->set(false);
    
    sound->play(A, 500);
    led->turnOn();
    shutter->turnOn(); 
    delay(INTERVAL_B);
    shutter->turnOff();
    led->turnOff();
    delay(500);
    sound->play(A, 500);

    listener->set(true);
    
}

void endSignal(){

     Serial.println("Finishing");

     for(int s = 0; s < 3; s++){

          led->turnOn();
          sound->play(A, 500);
          led->turnOff();
          delay(200);
     }

     led->turnOn();

     //while(RUNNING) { resetAndroid(); loop(); }
     //Serial.println("Full stop");

}

String parseData(String data_){

   String outputs[2];
   int starts = data_.indexOf('[') + 1;
   int ends = data_.indexOf(']');
   String split = data_.substring(starts, ends); 

   if(split.indexOf("_") == -1){ 
    if(split == "START") { scenarioA(); }
    else if(split == "SINGLE") { singleShot(); }
    else if(split == "RESETED") { return "Android app was reseted after timeout"; RUNNING = false; }
    else { return "unknown command"; }
   }
   else{
   String command = split.substring(0, split.indexOf("_"));
   String value =  split.substring(split.indexOf("_") + 1);
   if(command == "CCW") { motorA->moveNumOfSteps(value.toInt(), CCW); } 
   else if(command == "CW") { motorA->moveNumOfSteps(value.toInt(), CW); }
   else if(command == "FLASH" && value == "ON") { return "flash is on"; }
   else if(command == "FLASH" && value == "OFF") { return "flash is off"; }
   else if(command == "PARAMETER-A") { INTERVAL_A = value.toInt(); }
   else if(command == "PARAMETER-B") { INTERVAL_B = value.toInt(); }
   else if(command == "SHIFTS") { SHIFTS = value.toInt(); }
   else { return "unknow composite command"; }
   }

   
}

void listenBluetooth(){

 while (BT_HC06.available() && listener->get() == true) {
    
     delay(3);  
     char c = BT_HC06.read();
     metadata += c; 
     
   }
   
  if (metadata.length() > 0) { Serial.println(metadata); parseData(metadata); }

  delay(5);

}

void resetAndroid(){ BT_HC06.print("RESET"); delay(250); }

void delayWithListener(int value_){

     
}
