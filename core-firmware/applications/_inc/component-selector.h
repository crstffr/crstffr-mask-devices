#ifndef ADG409
#include "lib-adg409.h"
#endif

#ifndef Selector_h
#define Selector_h

class Selector
{
    private:
        char* _name;
        ADG409 _chip;

    public:
        Selector(char* name, int pin1, int pin2);
        int getSelection();
        void select(int input);
        void mqtt(MqttMessage msg);
        void sendStatus();
};

Selector::Selector(char* name, int pin1, int pin2) : _chip(pin1, pin2) {
    _name = name;
    _chip.select(1);
}

void Selector::select(int input) {
    _chip.select(input);
}

int Selector::getSelection() {
    return _chip.getSelection();
};

void Selector::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "input", _chip.getSelection());
    }
}

void Selector::mqtt(MqttMessage msg) {

    if (!msg.isFor(_name)) { return; }

    mqttLog("SELECTOR/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isCommand("select")) {
        _chip.select(msg.intVal());
        return;
    }

}


#endif