#ifndef Relay_h
#define Relay_h

class Relay
{
    public:
        Relay(char* name, int pin);
        void mqtt(MqttMessage msg);
        void sendStatus();
        bool state();
        void open();
        void close();

    private:
        int _pin;
        char* _name;
        int _state;
};

Relay::Relay(char* name, int pin) {
    pinMode(pin, OUTPUT);
    _pin = pin;
    _state = 0;
    _name = name;
}

bool Relay::state() {
    return _state == 1;
}

void Relay::open() {
    digitalWrite(_pin, HIGH);
    _state = 1;
    sendStatus();
}

void Relay::close() {
    digitalWrite(_pin, LOW);
    _state = 0;
    sendStatus();
}

void Relay::sendStatus() {
    if (IS_CONNECTED) {
        char state[3] = "";
        strcpy(state, (_state) ? "open" : "close");
        mqttStatus(_name, "state", state);
    }
}

void Relay::mqtt(MqttMessage msg) {

    if (!msg.isFor(_name)) { return; }

    mqttLog("RELAY/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isCommand("open")) {
        open();
    }

    if (msg.isCommand("close")) {
        close();
    }

}

#endif
