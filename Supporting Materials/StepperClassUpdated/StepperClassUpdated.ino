long NUM_OF_IMPULSES_PER_TURN = 32000;
int GEARBOX = 1;
int SHIFTS = 8;

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
 int fullTurn = 400; //200 by default, 400 high resolution motorp

public:
 StepperMotor(int dir_, int pull_, int enable_, int resolution_);
 void inits();
 void stepCW();   //один шаг по часовой стрелки
 void stepCCW();  //один шаг против часовой стрелки
 void moveNumOfSteps(int steps_, int dir_);
 void shift(int dir_);  
 void disable();  
 void enable();  
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

void StepperMotor::moveNumOfSteps(int steps_, int dir_){ digitalWrite(ports[2], LOW); for(int s = 0; s < steps_; s++){ if(dir_ == 1) { stepCW(); } else { stepCCW(); }} digitalWrite(this->ports[1], LOW); digitalWrite(ports[2], HIGH); }
void StepperMotor::shift(int dir_){ digitalWrite(ports[2], LOW); long steps = floor(NUM_OF_IMPULSES_PER_TURN / SHIFTS); for(int s = 0; s < steps; s++){ if(dir_ == 1) { stepCW(); } else { stepCCW(); }} digitalWrite(this->ports[1], LOW); digitalWrite(ports[2], HIGH); }
void StepperMotor::enable(){ digitalWrite(ports[2], HIGH); }
void StepperMotor::disable(){ digitalWrite(ports[2], LOW); }

StepperMotor *motorA; 
void setup() {

  Serial.begin(9600);
  motorA = new StepperMotor(9, 8, 7, 32);
  motorA->inits();
  

}

void loop() {

Serial.println("disabling");
motorA->disable();
delay(3000);
Serial.println("enabling");
motorA->enable();
delay(3000);

//  for(int i = 0; i <= SHIFTS; i++){
//  delay(2500);
//  motorA->shift(CW);
//  Serial.println(NUM_OF_IMPULSES_PER_TURN);
//  }
//
//    for(int i = 0; i <= SHIFTS; i++){
//  delay(2500);
//  motorA->shift(CCW);
//  Serial.println(NUM_OF_IMPULSES_PER_TURN);
//  }
  
}
