#include "application.h"
#include "lib-callback.h"
#include <math.h>

#ifndef Adafruit_MCP4725
#include "lib-mcp4725.h"
#endif

#ifndef Dac_h
#define Dac_h

#define MAX_NUMBER_OF_DAC_EVENTS (3)

class Dac
{
    public:
        Dac(char* name, uint8_t addr, int bitDepth, float refVoltage);
        void onChange(void (*callback)(void*), void* context);
        void setValue(int val, bool writeEEPROM);
        void mqtt(MqttMessage msg);
        void setValue(int val);
        void sendStatus();
        int getValue();
        void begin();
    private:
        int _value;
        char* _name;
        uint8_t _addr;

        int _resolution;
        float _refVoltage;
        float _voltageStep;
        Adafruit_MCP4725 _dac;

    protected:
      Callback _events[MAX_NUMBER_OF_DAC_EVENTS];
      void triggerChange();
};

Dac::Dac(char* name, uint8_t addr, int bitDepth, float refVoltage) : _dac(addr) {
    _value = 0;
    _name = name;
    _addr = addr;

    _refVoltage = refVoltage;
    _resolution = pow(2.0, bitDepth) - 1;
    _voltageStep = _refVoltage / _resolution;

    _dac.begin();
    _dac.setValue(_value);
}

void Dac::setValue(int val, bool writeEEPROM) {
    val = (val < 0) ? 0 : val;
    val = (val > _resolution) ? _resolution : val;
    _dac.setValue(val, writeEEPROM);
    _value = val;
    triggerChange();
}

void Dac::setValue(int val) {
    setValue(val, false);
}

int Dac::getValue() {
    return _value;
}

void Dac::onChange(void (*callback)(void*), void* context) {
    for (int i = 0; i < MAX_NUMBER_OF_DAC_EVENTS; i++) {
        if (_events[i].set == 0) {
            _events[i].callback = callback;
            _events[i].context = context;
            _events[i].set = 1;
            continue;
        }
    }
}

void Dac::triggerChange() {
    for (int i = 0; i < MAX_NUMBER_OF_DAC_EVENTS; i++) {
        if (_events[i].set == 1) {
            _events[i].execute();
        }
    }
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