#ifndef PIR_h
#define PIR_h

class PIR
{
    public:
        PIR(int pin);
        char motion();
    private:
        int _pin;
        int _val;
        bool _motion;
        unsigned long _now;
        unsigned long _last;
};

PIR::PIR(int pin) {
    _pin = pin;
}

void PIR::loop() {
    _now = millis();



}

char PIR::motion() {
    return digitalRead(_pin) == HIGH;
}

#endif
