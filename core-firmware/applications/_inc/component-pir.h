#ifndef PIR_h
#define PIR_h

class PIR
{
    public:
        PIR(char* name, int pin);
        bool state();
        void loop();
    private:
        int _pin;
        char* _name;
        int _delay;
        unsigned long _now;
        unsigned long _last;
};

PIR::PIR(char* name, int pin) {
    pinMode(pin, INPUT_PULLDOWN);
    _pin = pin;
    _now = 0;
    _last = 0;
    _name = name;
    _delay = 10000;
}

bool PIR::state() {
    return digitalRead(_pin) == HIGH;
}

void PIR::loop() {
    _now = millis();

    if (IS_CONNECTED) {
        if (this->state() && (_now > _last + _delay)) {
            mqttAction(_name, "motion");
            _last = _now;
        }
    }
}

#endif
