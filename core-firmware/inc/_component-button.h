#ifndef Button_h
#define Button_h

class Button
{
    public:
        Button(int pin, PinMode mode);
        char state();
    private:
        int _pin;
        int _val;
        bool _on;
        bool _off;
        bool _down;
        long _hold;
        bool _held;
        unsigned long _now;
        unsigned long _timer;
};

Button::Button(int pin, PinMode mode) {
    pinMode(pin, mode);
    _val = 0;
    _pin = pin;
    _hold = 2000;
    _held = false;
    _down = false;
    if (mode == INPUT_PULLUP) {
        _on = LOW;
        _off = HIGH;
    } else {
        _on = HIGH;
        _off = LOW;
    }
    digitalWrite(pin, _off);
}

char Button::state() {
    char out = '-';
    _now = millis();

    _val = digitalRead(_pin);

    if (!_down && !_held && _val == _on) {
        _down = true;
        _timer = millis();
    } else if (_down && !_held && _val == _on) {
        if (_now > _timer + _hold) {
            out = 'H';
            _held = true;
        }
    } else if (_val == _off) {
        if (_down && !_held) {
            out = 'P';
        }
        _down = false;
        _held = false;
    }
    return out;
}

#endif
