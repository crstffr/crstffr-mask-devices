#include "application.h"
#include "_led.h"

LED::LED(int pinR, int pinG, int pinB) {
    _pinR = pinR;
    _pinG = pinG;
    _pinB = pinB;
    _r = 0;
    _g = 0;
    _b = 0;
    _state = 0;
    _blink = 0;
    _blinkGap = 350;
    _intensity = 0;
    _intensityMax = 30;
    pinMode(_pinR, OUTPUT);
    pinMode(_pinG, OUTPUT);
    pinMode(_pinB, OUTPUT);
}

void LED::loop() {
    _now = millis();
    if (_blinking) {
        calcBlink();
    }
    if (_state == 1) {
        change();
    }
}

void LED::on() {
    _state = 1;
}

void LED::off() {
    _state = 0;
    blank();
}

void LED::change() {
    intensity(_intensity);
}

void LED::blank() {
    analogWrite(_pinR, 0);
    analogWrite(_pinG, 0);
    analogWrite(_pinB, 0);
}

void LED::rgb(int r, int g, int b) {
    _r = r;
    _g = g;
    _b = b;
}

void LED::dim() {
    if (_state == 1) {
        if (_intensity == 0) {
            intensity(_intensityMax);
        } else if (_intensity > 5) {
            intensity(5);
        } else {
            intensity(0);
        }
    }
}

void LED::intensity(int value) {
    if (_state == 1) {
        _intensity = value;
        int r = int((_r / 100) * value);
        int g = int((_g / 100) * value);
        int b = int((_b / 100) * value);
        analogWrite(_pinR, r);
        analogWrite(_pinG, g);
        analogWrite(_pinB, b);
    }
}

void LED::setMaxIntensity(int value) {
    _intensityMax = value;
    _intensity = value;
}

void LED::fade() {
    _fading = true;
    _blinking = false;
}

void LED::blink() {
    _fading = false;
    _blinking = true;
    _blinkTimer = _now + _blinkGap;
}

void LED::calcBlink() {
    if (_now > _blinkTimer) {
        if (_blink == 0) {
            blank();
        } else {
            rgb(_r,_g,_b);
        }
        _blink = (_blink == 0) ? 1 : 0;
        _blinkTimer = _now + _blinkGap;
    }
}

char* LED::getColor() {
    return _color;
}

int LED::getIntensity() {
    return _intensity;
}

int LED::getState() {
    return _state;
}

void LED::color(char* color) {

    _fading = false;
    _blinking = false;
    _color = color;

    if (strcmp(color, "white") == 0) {
        rgb(255, 255, 255);
    }

    if (strcmp(color, "red") == 0) {
        rgb(255, 0, 0);
    }

    if (strcmp(color, "green") == 0) {
        rgb(0, 255, 0);
    }

    if (strcmp(color, "blue") == 0) {
        rgb(0, 0, 255);
    }

    if (strcmp(color, "cyan") == 0) {
        rgb(0, 255, 255);
    }

    if (strcmp(color, "magenta") == 0) {
        rgb(255, 0, 255);
    }

    if (strcmp(color, "yellow") == 0) {
        rgb(255, 255, 0);
    }

    if (strcmp(color, "orange") == 0) {
        rgb(255, 165, 0);
    }

    change();

}