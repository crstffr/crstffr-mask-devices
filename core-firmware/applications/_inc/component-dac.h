#include "application.h"

#ifndef Adafruit_MCP4725
#include "lib-mcp4725.h"
#endif

#ifndef Dac_h
#define Dac_h

class Dac
{
    public:
        Dac(char* name, uint8_t addr);
        void mqtt(MqttMessage msg);
        void setValue(int val, bool writeEEPROM);
        void setValue(int val);
        void sendStatus();
        int getValue();
        void begin();
    private:
        int _value;
        char* _name;
        uint8_t _addr;
        Adafruit_MCP4725 _dac;
};

Dac::Dac(char* name, uint8_t addr) : _dac(addr) {
    _name = name;
    _addr = addr;
}

void Dac::begin() {
    _dac.begin();
}

void Dac::setValue(int val, bool writeEEPROM) {
    val = (val < 0) ? 0 : val;
    val = (val > 4095) ? 4095 : val;
    _dac.setValue(val);
    _value = val;
}

void Dac::setValue(int val) {
    setValue(val, false);
    sendStatus();
}

int Dac::getValue() {
    return _value;
}

void Dac::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "value", _value);
    }
}

void Dac::mqtt(MqttMessage msg) {

    if (!msg.isFor(_name)) { return; }

    mqttLog("Dac/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isCommand("value")) {
        setValue(msg.intVal());
    }

}

#endif