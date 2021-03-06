#ifndef Button_h
#define Button_h

class Button
{
    public:
        Button(char* name, int pin, PinMode mode);
        void onPress(callback fn);
        void onDown(callback fn);
        void onHold(callback fn);
        void onUp(callback fn);
        char state();
        void loop();
    private:
        int _pin;
        int _val;
        bool _on;
        bool _off;
        bool _down;
        long _hold;
        bool _held;
        char* _name;
        unsigned long _now;
        unsigned long _timer;
        callback _onPress;
        callback _onDown;
        callback _onHold;
        callback _onUp;
};

Button::Button(char* name, int pin, PinMode mode) {

    pinMode(pin, mode);
    _pin = pin;
    _name = name;
    _val = 0;
    _hold = 2000;
    _held = false;
    _down = false;

    _onPress = noop;
    _onDown = noop;
    _onHold = noop;
    _onUp = noop;

    if (mode == INPUT_PULLUP) {
        _on = LOW;
        _off = HIGH;
    } else {
        _on = HIGH;
        _off = LOW;
    }

    digitalWrite(pin, _off);
}

void Button::onPress(callback fn) {
    _onPress = fn;
}

void Button::onDown(callback fn) {
    _onDown = fn;
}

void Button::onHold(callback fn) {
    _onHold = fn;
}

void Button::onUp(callback fn) {
    _onUp = fn;
}

char Button::state() {
    char out = '-';
    _now = millis();

    _val = digitalRead(_pin);

    if (!_down && !_held && _val == _on) {
        out = 'D';
        _down = true;
        _timer = millis();
    } else if (_down && !_held && _val == _on) {
        if (_now > _timer + _hold) {
            out = 'H';
            _held = true;
        } else {
            out = 'D';
        }
    } else if (_val == _off) {
        if (_down && !_held) {
            out = 'P';
        } else if (_down) {
            out = 'U';
        }
        _down = false;
        _held = false;
    }
    return out;
}


void Button::loop() {

    switch (this->state()) {
        case 'D':
            _onDown();
            break;

        case 'U':
            _onUp();
            break;

        case 'P':
            _onPress();
            if (IS_CONNECTED) {
                mqttAction(_name, "press");
            }
            break;

        case 'H':
            _onHold();
            if (IS_CONNECTED) {
                mqttAction(_name, "hold");
            }
            break;
    }
}

#endif
