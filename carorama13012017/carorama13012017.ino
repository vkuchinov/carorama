/*
    УПРАВЛЕНИЕ КАМЕРОЙ ПО ОДНОЙ ОСЯМ v.0.9β

                                        ПОРТ            ПИТАНИЕ      DEBOUNCING [ДРЕБЕЗЖАНИЕ]     ШАГ

    1 x Bluetooth HC-06 модуль          D1, D2, GND     5В           -
    1 х пьезодатчик RobotDyn            D12             5В           -
    1 x реле DPDT                       D5,             5В           -
    1 x реле OMIRON для вспышки         D6                           -
    1 x светодиод на 220Ω               D13             5В           -
    1 x NEMA 1X двигатель на SD8825     D8, D9, D7      5В/12В       -                            1/32

    1 x голубой светодиод на 220Ω       D11             5В           для индикации работы Bluetooth

    1 x RESET кнопка                    RST             5В           -

    REFERENCES:
    http://playground.arduino.cc/Code/Bounce [debounce library]
    http://ai2.appinventor.mit.edu/
    https://github.com/carlosmaniero/Arduino-Async-Delay/blob/master/examples/01_hello.ino
    http://www.makeuseof.com/tag/arduino-delay-function-shouldnt-use/

    http://www.martyncurrey.com/arduino-with-hc-05-bluetooth-module-at-mode/
    https://learn.sparkfun.com/tutorials/using-the-bluesmirf

    LATEST ANDROID BUILD
    http://ai2.appinventor.mit.edu/?locale=en#6339780298932224

    @author   Vladimir V. KUCHINOV
    @email    helloworld@vkuchinov.co.uk

*/

/*
       DRV8825 BOARD SCHEME

              B R K G
              | | | |
 *     *********************
 *     *            *****  *   G: GREEN
 *     *            *   *  *   R: RED
 *     *   DRV8825  *   *  *   B: BLUE
 *     *            *   *  *   K: BLACK
 *     *            *****  *
 *     *********************

*/

/*
    LISTENER

    1. NOT LISTENING WHILE DOING SOMETHING


*/


#include<SoftwareSerial.h>

SoftwareSerial BT_HC06(2, 3); // TX, RX

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
   BUZZ CLASS: ПЬЕЗОДИНАМИК
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

Buzz::Buzz(int port_) {
  this->port = port_;
}
void Buzz::inits() {
  pinMode(this->port, OUTPUT);
  delay(50);
  Serial.println("Sound is initialized.");
}
void Buzz::play(int note_, int length_) {


  //C, D, E, F, G, A, B
  int notes[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014 };
  int timeHigh = notes[note_];

  for (int i = 0; i < length_; i++) {
    digitalWrite(this->port, HIGH);
    delayMicroseconds(timeHigh);
    digitalWrite(this->port, LOW);
    delayMicroseconds(timeHigh);
  }
}

/*******************************
   LED CLASS: СВЕТОДИОД
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

LED::LED(int port_) {
  this->port = port_;
}
void LED::inits() {
  pinMode(this->port, OUTPUT);
  delay(50);
  Serial.println("LED is initialized.");
}
void LED::turnOn() {
  digitalWrite(this->port, HIGH);
}
void LED::turnOff() {
  digitalWrite(this->port, LOW);
}

/***********************************
   RELAY CLASS: Реле одиночное
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

Relay::Relay(int port_) {
  this->port = port_;
}
void Relay::inits() {
  pinMode(this->port, OUTPUT);
  delay(250);
  Serial.println("Relay is initialized.");
}
void Relay::turnOn() {
  if (this->enabled) {
    digitalWrite(this->port, HIGH);
  }
}
void Relay::turnOff() {
  digitalWrite(this->port, LOW);
}

/*************************************************************************
   LISTENER CLASS: FOR SMOOTH & STABLE ARDUINO/ANDROID COMMUNICATION
 ************************************************************************/

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
    BT_HC06.print("![IDLE]");
    this->getOK();
  } this->set(true);
  handshaked = false;

  delay(750);
  
}

void Listener::sendIdle(int times_) { BT_HC06.print("![IDLE]"); }

void Listener::getOK() {

  String metadata = "";

  while (BT_HC06.available()) {

    delay(3);
    char c = BT_HC06.read();
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

  while (BT_HC06.available()) {

    delay(3);
    char c = BT_HC06.read();
    metadata += c;

  }

  if (metadata.length() > 0 && metadata.indexOf("STOP") >= 0) {
    this->handshaked = true;
  }

}

/*******************************************************************
   STEPPER MOTOR CLASS: ШАГОВЫЙ ДВИГАТЕЛЬ ЧЕРЕЗ SD8825
 ******************************************************************/

#define CW   1
#define CCW -1

class StepperMotor {
  private:

    int dir = 1;
    int pause = 3;
    int micropause = 2;
    int resolution; //1: 1/1, 2: 1/2, 4: 1/4, 8: 1/8, 16: 1/16...
    int fullTurn = MOTOR_RESOLUTION; //200 by default, 400 high resolution motorp

  public:
    StepperMotor(int dir_, int pull_, int enable_, int resolution_);

    int ports[3]; //0: dir, 1: pull, 2: enableint ports[3]; //0: dir, 1: pull, 2: enable
    void inits();
    void stepCW();   //один шаг по часовой стрелки
    void stepCCW();  //один шаг против часовой стрелки
    void moveNumOfSteps(int steps_, int dir_);
    void shift(int dir_);
    void disable();
    void enable();
};

StepperMotor::StepperMotor(int dir_, int pull_, int enable_, int resolution_) {
  this->ports[0] = dir_;
  this->ports[1] = pull_;
  this->ports[2] = enable_;
  resolution = resolution_;
}
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

void StepperMotor::stepCW() {

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

void StepperMotor::stepCCW() {

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

void StepperMotor::moveNumOfSteps(int steps_, int dir_) {
  digitalWrite(ports[2], LOW);
  for (int s = 0; s < steps_; s++) {
    if (dir_ == 1) {
      stepCW();
    } else {
      stepCCW();
    }
  } digitalWrite(this->ports[1], LOW);
  digitalWrite(ports[2], HIGH);
}
void StepperMotor::shift(int dir_) {
  digitalWrite(ports[2], LOW);
  long steps = floor(NUM_OF_IMPULSES_PER_TURN / SHIFTS);
  for (int s = 0; s < steps; s++) {
    if (dir_ == 1) {
      stepCW();
    } else {
      stepCCW();
    }
  } digitalWrite(this->ports[1], LOW);
  digitalWrite(ports[2], HIGH);
}
void StepperMotor::enable() {
  digitalWrite(ports[2], LOW);
}
void StepperMotor::disable() {
  digitalWrite(ports[2], HIGH);
}


LED *led;
Buzz *sound;
Relay *flash;
Relay *shutter;

StepperMotor *motorA;

Listener *listener;


void setup() {

  Serial.begin(9600);
  Serial.println("Starting");

  BT_HC06.begin(115200);
  BT_HC06.print("$");
  BT_HC06.print("$");
  BT_HC06.print("$");
  delay(100);
  BT_HC06.println("U,9600,N");
  BT_HC06.begin(9600);

  //Listener
  listener = new Listener(11);
  listener->sendIdle(9); 
  
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
  motorA->enable();

  delay(500);
  for (int i = 0; i < 4; i++) {
    motorA->shift(CW); //test
  }
  delay(1500);
  for (int i = 0; i < 4; i++) {
    motorA->shift(CCW); //test
  }
  delay(1500);
  shutter->turnOn();
  led->turnOn();
  delay(1500);
  shutter->turnOff();
  led->turnOff();

  listener->set(true);

  //мотор свободный для вращения
  Serial.println("motor disabled");
  motorA->disable();

}

void loop() {
  
  listenBluetooth();
  listener->sendIdle();
}

void scenarioA() {

  //мотор становится активным
  motorA->enable();

  Serial.println("Main Sequence");
  RUNNING = true;

  led->turnOff();
  sound->play(A, 2000);
  delay(1000);

  for (int i = 0; i < SHIFTS; i++) {

    shutter->turnOn();
    delay(INTERVAL_B);
    shutter->turnOff();
    motorA->shift(CW);
    delay(INTERVAL_A);
  }

  delay(1500);

  endSignal();

  //мотор свободный для вращения
  Serial.println("motor disabled");
  motorA->disable();

}

void singleShot() {

  sound->play(A, 500);
  led->turnOn();
  shutter->turnOn();
  delay(INTERVAL_B);
  shutter->turnOff();
  led->turnOff();
  delay(500);
  sound->play(A, 500);

}

void endSignal() {

  Serial.println("Finishing");

  for (int s = 0; s < 3; s++) {

    led->turnOn();
    sound->play(A, 500);
    led->turnOff();
    delay(200);
  }

  led->turnOn();

  //while(RUNNING) { resetAndroid(); loop(); }
  //Serial.println("Full stop");

}

String parseData(String data_) {

  if (data_.indexOf('_') < 0) {
    if (data_.indexOf("STR") >= 0) {
      listener->set(false);
      scenarioA();
      listener->sendIdle();
    }
    else if (data_.indexOf("SNG") >= 0) {
      listener->set(false);
      singleShot();
      listener->sendIdle();
    }
    else if (data_.indexOf("FLSN") >= 0) {
      listener->set(false);
      Serial.println("flash on");
      listener->sendIdle();
    }
    else if (data_.indexOf("FLSF") >= 0) {
      listener->set(false);
      Serial.println("flash off");
      listener->sendIdle();
    }
    else {
      return "[UNKNOWN COMMAND]";
    }
  }
  else {
    //String command = data_.substring(0, split.indexOf("_"));
    //String value =  data_.substring(split.indexOf("_") + 1);
    String value = GetOnlyNumbers(data_.substring(data_.indexOf("_") + 1));


    if (data_.indexOf("CW") >= 0) {
      listener->set(false);
      int val = round(NUM_OF_IMPULSES_PER_TURN / 360.0 * value.toInt());
      motorA->moveNumOfSteps(val, CCW);
      listener->sendIdle();
    }
    else if (data_.indexOf("CL") >= 0 ) {
      listener->set(false);
      int val = round(NUM_OF_IMPULSES_PER_TURN / 360.0 * value.toInt());
      motorA->moveNumOfSteps(val, CW);
      listener->sendIdle();
    }
    else if (data_.indexOf("ST") >= 0) {
      listener->set(false);
      Serial.println(value);
      INTERVAL_A = value.toFloat() * 1000;
      listener->sendIdle();
    }
    else if (data_.indexOf("EXP") >= 0) {
      listener->set(false);
      Serial.println(value);
      INTERVAL_B = value.toFloat() * 1000;
      listener->sendIdle();
    }
    else if (data_.indexOf("SLZ") >= 0) {
      listener->set(false);
      Serial.println(value);
      SHIFTS = value.toInt();
      listener->sendIdle();
    }
    else {
      return "[UNKNOWN COMPOSITE COMMAND]";
    }
  }

}

String GetOnlyNumbers(String source_) {

  String output = "";
  for (int c = 0; c < source_.length(); c++) {
    if ((byte)source_.charAt(c) > 47 && (byte)source_.charAt(c) < 58) { output += source_.charAt(c); }
    else if(source_.charAt(c) == '.') { output += source_.charAt(c); }
  }

  return output;
}

void listenBluetooth() {

  String metadata = "";

  while (BT_HC06.available() && listener->state == true) {

    delay(2);
    char c = BT_HC06.read();
    metadata += c;

  }

  if (metadata.length() > 0) {
    Serial.println(metadata);
    parseData(metadata);
  }

  delay(5);

}

