/*
 *  УПРАВЛЕНИЕ КАМЕРОЙ ПО ОДНОЙ ОСЯМ v.0.5α
 * 
 *                                      ПОРТ            ПИТАНИЕ      DEBOUNCING [ДРЕБЕЗЖАНИЕ]     ШАГ               
 *                                        
 *  1 x Bluetooth HC-06 модуль          D1, D2, GND     5В           -
 *  1 х пьезодатчик RobotDyn            D12             5В           -
 *  1 x релейная пара RobotDyn          D5,             5В           -
 *  1 x реле OMIRON для вспышки         D6                           -
 *  1 x светодиод на 220Ω резисторе     D13             5В           -
 *  1 x NEMA 1X двигатель на SD8825     D8, D9          5В/12В       -                            1/32
 *  
 *  1 x RESET кнопка                    RST             5В           -
 *  
 *  REFERENCES:
 *  http://playground.arduino.cc/Code/Bounce [debounce library]
 *  http://ai2.appinventor.mit.edu/
 *  https://github.com/carlosmaniero/Arduino-Async-Delay/blob/master/examples/01_hello.ino
 *  http://www.makeuseof.com/tag/arduino-delay-function-shouldnt-use/
 *  
 *  @author   Vladimir V. KUCHINOV
 *  @email    helloworld@vkuchinov.co.uk
 *  
 */

#include <Bounce2.h>
#include <SoftwareSerial.h>

SoftwareSerial BT_HC06(0, 1); // RX, TX
String metadata;

//VARIALBES

#define STRSWITCH(STR)      char _x[16]; strcpy(_x, STR); if (false) 
#define STRCASE(STR)        } else if (strcmp(_x, STR)==0){
#define STRDEFAULT          } else {

bool RUNNING = false;

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
 bool enabled = false;
};

Relay::Relay(int port_) { this->port = port_; }
void Relay::inits() { pinMode(this->port, OUTPUT); delay(50); Serial.println("Relay is initialized."); }
void Relay::turnOn() { if(this->enabled) { digitalWrite(this->port, HIGH); } }
void Relay::turnOff() { digitalWrite(this->port, LOW); } 

/*******************************************************************
 * STEPPER MOTOR CLASS: ШАГОВЫЙ ДВИГАТЕЛЬ ЧЕРЕЗ SD8825 
 ******************************************************************/

#define CW   1
#define CCW -1

class StepperMotor {
private:

 int ports[3]; //0: dir, 1: pull, 2: enable
 int dir = 1;
 int pause = 1;
 int resolution; //1: 1/1, 2: 1/2, 4: 1/4, 8: 1/8, 16: 1/16...
 int fullTurn = 200; //200 by default, 400 high resolution motor

public:
 StepperMotor(int dir_, int pull_, int enable_, int resolution_);
 void inits();
 void stepCW();   //один шаг по часовой стрелки
 void stepCCW();  //один шаг против часовой стрелки
 void moveNumOfSteps(int steps_, int dir_);
 void moveByAngle(float angle_);  
};

StepperMotor::StepperMotor(int dir_, int pull_, int enable_, int resolution_) { this->ports[0] = dir_; this->ports[1] = pull_; this->ports[2] = enable_; resolution = resolution_; }
void StepperMotor::inits() { pinMode(this->ports[0], OUTPUT);  pinMode(this->ports[1], OUTPUT); pinMode(this->ports[2], OUTPUT); digitalWrite(this->ports[2], HIGH), delay(50); Serial.println("Stepper is initialized."); }
void StepperMotor::stepCW(){

    digitalWrite(this->ports[0], true); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(2);
    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(2);
 
    digitalWrite(this->ports[0], false); 
    delayMicroseconds(pause);
    
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(2);
    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(2);
  
}

void StepperMotor::stepCCW(){

    digitalWrite(this->ports[0], true); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(2);
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(2);
 
    digitalWrite(this->ports[0], false); 
    delayMicroseconds(pause);

    digitalWrite(this->ports[1], HIGH);
    delayMicroseconds(2);
    digitalWrite(this->ports[1], LOW);
    delayMicroseconds(2);
  
}

void StepperMotor::moveNumOfSteps(int steps_, int dir_){ for(int s = 0; s < steps_; s++){ if(dir_ == 1) { stepCW(); } else { stepCCW(); }} digitalWrite(this->ports[1], LOW);  }
void StepperMotor::moveByAngle(float angle_){ 

     if(angle_ != 0){
       int steps = fullTurn * resolution / 360.0 * abs(angle_);
       for(int s = 0; s < steps; s++){ if(angle_ > 0) { stepCW(); } else { stepCCW(); }} 
       digitalWrite(this->ports[1], LOW);  
     }
     
}

/*************************************************************************
 * TIMER CLASS: USE IT INSTEAD OF 'BLOCKING' DELAY() / DELAYMICROSECONDS()
 ************************************************************************/

class Timer {
private:
 long currentTime;
 
public:
 Timer();
 void inits();
 void setToZero();
 void setDelay();
};

Timer::Timer() { currentTime = millis(); }
void Timer::inits() { Serial.println("Timer is initialized."); }
void Timer::setToZero() {  }
void Timer::setDelay() {  } 



LED *led;
Buzz *sound;
Relay *flash;
Relay *shutter;

StepperMotor *motorA;

void setup() {

  Serial.begin(9600);

  BT_HC06.begin(9600);
  BT_HC06.println("BT_HC06 module is ready");
  
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
  flash = new Relay(6);
  flash->inits();
  
  //steppers
  motorA = new StepperMotor(8, 9, 7, 32);
  motorA->inits();

  motorA->moveNumOfSteps(32, CCW);
  motorA->moveNumOfSteps(32, CW);

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
    STRCASE ("STPR1_CCW") 
    Serial.println("turn left");
    if(!RUNNING) { motorA->moveNumOfSteps(4, CCW); }
    STRCASE ("STPR1_CW") 
    Serial.println("turn right");
    if(!RUNNING) { motorA->moveNumOfSteps(4, CW); }
    STRCASE ("START") 
    if(!RUNNING) { scenarioA(); }
    STRCASE ("SHUTTER") 
    if(!RUNNING) { }
    STRCASE ("STOP") 
    asm volatile ("  jmp 0"); 
    STRCASE ("FLASH_ON") 
    if(RUNNING) { flash->enabled = true; }
    STRCASE ("FLASH_OFF") 
    if(RUNNING) { flash->enabled = false; }
    STRDEFAULT 
      Serial.println("unknown command");
  }
  metadata = "";
}

}

void scenarioA(){

     Serial.println("Scenario A");
     RUNNING = true;

     led->turnOff();
     sound->play(A, 2000);
     delay(10000);

     for(int i = 0; i < 10; i++){

           shutter->turnOn(); 
           delay(10000);
           motorA->moveNumOfSteps(8, CW);

     }

     //motorB->moveNumOfSteps(8, CW);
     delay(1500);

     endSignal();

     RUNNING = false;
  
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

     Serial.println("Full stop");
     
}

void delayWithBluetoothListener(int millis_){



}

