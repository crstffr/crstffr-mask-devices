#ifndef LED_h
#define LED_h

class LED
{
    public:
        LED(char* name, int pinR, int pinG, int pinB);
        void mqtt(MqttMessage msg);
        void on();
        void off();
        void loop();
        void rgb(int r, int g, int b);
        void intensity(int value);
        void setMaxIntensity(int value);
        void color(char* name);
        void dim();
        void blank();
        void blink(bool state);
        void blinking();
        void change();
        void calcFade();
        void calcBlink();
        int getState();
        int getIntensity();
        char* getColor();
        void sendStatus();
        void sendState();
        void sendColor();
        void sendIntensity();

    private:
        int _pinR;
        int _pinG;
        int _pinB;
        int _r;
        int _g;
        int _b;
        int _state;
        char* _name;
        int _blinkState;
        int _blinkTimer;
        char* _color;
        int _intensity;
        int _intensityMax;
        unsigned long _now;
        unsigned long _blinkGap;

        SimpleTimer timer;
};


LED::LED(char* name, int pinR, int pinG, int pinB) {
    _pinR = pinR;
    _pinG = pinG;
    _pinB = pinB;
    _r = 0;
    _g = 0;
    _b = 0;
    _state = 0;
    _name = name;
    _blinkGap = 350;
    _blinkState = 0;
    _intensity = 0;
    _intensityMax = 30;
    pinMode(_pinR, OUTPUT);
    pinMode(_pinG, OUTPUT);
    pinMode(_pinB, OUTPUT);

    //_blinkTimer = timer.setInterval(_blinkGap, blinking);
    //timer.disable(_blinkTimer);
}

void LED::loop() {
    timer.run();
    if (_state == 1 && _blinkState == 0) {
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
        if (_intensity <= 1) {
            intensity(_intensityMax);
        } else {
            intensity(_intensity / 2);
        }
    }
}

void LED::setMaxIntensity(int value) {
    _intensityMax = value;
}

void LED::intensity(int value) {
    if (value != _intensity) {
        _intensity = value;
        sendIntensity();
    }
    if (_state == 1) {
        int r = int((_r / 100) * value);
        int g = int((_g / 100) * value);
        int b = int((_b / 100) * value);
        analogWrite(_pinR, r);
        analogWrite(_pinG, g);
        analogWrite(_pinB, b);
    }
}

void LED::blink(bool state) {
    if (state) {
        timer.enable(_blinkTimer);
    } else {
        timer.disable(_blinkTimer);
    }
}

void LED::blinking() {
    if (_blinkState == 1) {
        blank();
    } else {
        intensity(_intensity);
    }
    _blinkState = (_blinkState == 0) ? 1 : 0;
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

    if (color != _color) {
        _color = color;
        sendColor();
    }

    change();

}

void LED::sendStatus() {
    sendState();
    sendColor();
    sendIntensity();
}

void LED::sendColor() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "color", _color);
    }
}

void LED::sendState() {
    if (IS_CONNECTED) {
        char leds[3] = "";
        strcpy(leds, (_state == 1) ? "on" : "off");
        mqttStatus(_name, "state", leds);
    }
}

void LED::sendIntensity() {
    if (IS_CONNECTED) {
        char ledi[3] = "";
        itoa(_intensity, ledi, 10);
        mqttStatus(_name, "intensity", ledi);
    }
}


void LED::mqtt(MqttMessage msg) {

    if (!msg.isForMe(_name)) { return; }

    mqttLog("LED/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isSetup("maxintensity")) {
        setMaxIntensity(msg.intVal());
        return;
    }

    if (msg.isCommand("state")) {
        if (msg.is("on")) {
            on();
        } else if (msg.is("off")) {
            off();
        }
        return;
    }

    if (msg.isCommand("dim")) {
        dim();
        return;
    }

    if (msg.isCommand("color")) {
        color(msg.charVal());
        return;
    }

    if (msg.isCommand("intensity")) {
        intensity(msg.intVal());
        return;
    }

}


#endif