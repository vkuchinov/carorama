/*

#define LOGIC    0
#define PHYSICAL 1

class Joystick {
private:

 int ports[3];
 int axisDibounce = 16; //значение

 int buttonType;
 int buttonState;
 int lastButtonState = LOW;
 long buttonDebounce = 50;   //миллисекунды
 long lastButtonDebounceTime = 0;

 Bounce debouncer;

public:
 Joystick(int axisX_, int axisY_, int button_);
 void inits();
 void update();
 int getValueX();
 int getValueY();
 int mapValue(int value_);
 bool getButtonState();

};

Joystick::Joystick(int axisX_, int axisY_, int button_) { this->ports[0] = axisX_; this->ports[1] = axisY_; this->ports[2] = button_; Bounce debouncer = Bounce(); }
void Joystick::inits() { pinMode(this->ports[0], INPUT);  pinMode(this->ports[1], INPUT); pinMode(this->ports[2], INPUT_PULLUP);  debouncer.attach(this->ports[2]); debouncer.interval(10);  delay(50); Serial.println("Joystick is initialized."); }
void Joystick::update() { debouncer.update(); } 
int Joystick::mapValue(int value_) { return constrain(64, 512, map(value_, 0, 1024, -512, 512)); } 
int Joystick::getValueX() { return this->mapValue(analogRead(this->ports[0])); }
int Joystick::getValueY() { return this->mapValue(analogRead(this->ports[1])); }
bool Joystick::getButtonState(){ return debouncer.read(); }

class Button {
private:

 int port;

 int buttonType;
 Bounce debouncer;
 
public:
 Button(int port_);
 void inits();
 void update();
 bool getButtonState();
 
};

Button::Button(int port_) { this->port = port_; Bounce debouncer = Bounce(); }
void Button::inits() { pinMode(this->port, INPUT_PULLUP);  delay(50); Serial.println("Button is initialized."); debouncer.attach(this->port); debouncer.interval(10); }
void Button::update() { debouncer.update(); }
bool Button::getButtonState(){ return debouncer.read(); }

**********************************
* RELAYSTACK CLASS: Спаренные реле 
**********************************

class RelayStack {
private:
 int ports[2];
 
public:
 RelayStack(int portA_, int portB_);
 void inits();
 void turnOn(int port_);
 void turnOff(int port_);
 void setActive(int port_, int length_);
};

RelayStack::RelayStack(int portA_, int portB_) { this->ports[0] = portA_; this->ports[1] = portB_; }
void RelayStack::inits() { pinMode(this->ports[0], OUTPUT); pinMode(this->ports[1], OUTPUT); delay(50); Serial.println("Relays are initialized."); }
void RelayStack::turnOn(int port_) { digitalWrite(this->ports[port_], HIGH); }
void RelayStack::turnOff(int port_) { digitalWrite(this->ports[port_], LOW); } 
void RelayStack::setActive(int port_, int length_) { this->turnOn(port_); delay(length_); this->turnOff(port_);  }  

*/
