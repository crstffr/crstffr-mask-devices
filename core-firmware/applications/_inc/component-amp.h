#ifndef Amp_h
#define Amp_h

int AMP_VOLUME_LOW = 8;
int AMP_VOLUME_MED = 32;
int AMP_VOLUME_HIGH = 48;
int AMP_VOLUME_DEFAULT = 16;

class Amp
{
    public:
        Amp(char* name, int stbyPin, int mutePin, int volUpPin, int volDnPin);
        void setup();

        void powerOn();
        void powerOff();
        void powerToggle();

        void volumeUp();
        void volumeDown();
        void volumeReset();
        void volumeSet(int level);

        void onPowerOn(callback fn);
        void onPowerOff(callback fn);

        void sendState();
        void sendVolume();
        void sendStatus();

    private:
        int _stbyPin;
        int _mutePin;
        int _volUpPin;
        int _volDnPin;

        int _volume;
        bool _state;
        char* _name;

        callback _onPowerOn;
        callback _onPowerOff;
};

Amp::Amp(char* name, int stbyPin, int mutePin, int volUpPin, int volDnPin) {

    _name = name;
    _stbyPin = stbyPin;
    _mutePin = mutePin;
    _volUpPin = volUpPin;
    _volDnPin = volDnPin;

    _volume = 0;
    _state = false;
    _onPowerOn = noop;
    _onPowerOff = noop;

    pinMode(_volUpPin, OUTPUT);
    pinMode(_volDnPin, OUTPUT);
    pinMode(_stbyPin,  OUTPUT);

}

void Amp::setup() {
    digitalWrite(_volUpPin, LOW);
    digitalWrite(_volDnPin, LOW);
    delay(500);
    digitalWrite(_volUpPin, HIGH);
    digitalWrite(_volDnPin, HIGH);
}

void Amp::onPowerOn(callback fn) {
    _onPowerOn = fn;
}

void Amp::onPowerOff(callback fn) {
    _onPowerOff = fn;
}

void Amp::sendStatus() {
    this->sendState();
    this->sendVolume();
}

void Amp::sendState() {
    if (IS_CONNECTED) {
        char state[3] = "";
        strcpy(state, (_state) ? "on" : "off");
        mqttStatus(_name, "power", state);
    }
}

void Amp::sendVolume() {
    if (IS_CONNECTED) {
        char level[3] = "";
        itoa(_volume, level, 10);
        mqttStatus(_name, "volume", level);
    }
}

void Amp::powerOn() {
    if (!_state) {
        digitalWrite(_stbyPin, HIGH);
        this->volumeSet(AMP_VOLUME_DEFAULT);
        _state = true;
        this->sendStatus();
        this->_onPowerOn();
    }
}

void Amp::powerOff() {
    if (_state) {
        digitalWrite(_stbyPin, LOW);
        this->volumeSet(0);
        _state = false;
        this->sendStatus();
        this->_onPowerOff();
    }
}

void Amp::powerToggle() {
    if (_state) {
        this->powerOff();
    } else {
        this->powerOn();
    }
}

void Amp::volumeUp() {
    delay(5);
    digitalWrite(_volUpPin, LOW);
    delay(5);
    digitalWrite(_volUpPin, HIGH);
    if (_volume < 64) {
        _volume++;
    }
}

void Amp::volumeDown() {
    delay(5);
    digitalWrite(_volDnPin, LOW);
    delay(5);
    digitalWrite(_volDnPin, HIGH);
    if (_volume > 0) {
        _volume--;
    }
}

void Amp::volumeSet(int level) {
    if (level > _volume) {
        for (int i=_volume; i<level; i++) {
            this->volumeUp();
        }
    } else {
        for (int i=_volume; i>level; i--) {
            this->volumeDown();
        }
    }
}

void Amp::volumeReset() {
    for (int i=0; i<=100; i++) {
        this->volumeDown();
        delay(10);
    }
    _volume = 0;
}

#endif
