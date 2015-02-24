#include "application.h"
#include <math.h>

#ifndef Adc_h
#define Adc_h

class Adc
{
    public:
        Adc(char* name, int pin, int bitDepth, float refVoltage);
        void onChange(callback fn);
        void compare(int dacValue);
        void mqtt(MqttMessage msg);
        float getVoltage();
        void sendStatus();
        int getValue();
        void read();


    private:
        int _pin;
        int _value;
        char* _name;
        int _bitDepth;
        float _voltage;
        int _resolution;
        float _refVoltage;
        float _voltageStep;
        callback _onChange;
};

Adc::Adc(char* name, int pin, int bitDepth, float refVoltage) {
    _pin = pin;
    _value = 0;
    _voltage = 0;
    _name = name;
    _onChange = noop;
    _bitDepth = bitDepth;
    _refVoltage = refVoltage;
    _resolution = pow(2.0, bitDepth) - 1;
    _voltageStep = _refVoltage / _resolution;
}

void Adc::read() {
    int value = analogRead(_pin);
    if (value != _value) {
        _value = value;
        _voltage = _value * _voltageStep;
    }
}

int Adc::getValue() {
    read();
    return _value;
}

float Adc::getVoltage() {
    read();
    return _voltage;
}

void Adc::onChange(callback fn) {
    _onChange = fn;
}

void Adc::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "value", _value);
        mqttStatus(_name, "voltage", _voltage);
    }
}

void Adc::compare(int dacValue) {
    if (IS_CONNECTED) {
        String s = String(dacValue) + ":" + String(_value);
        char c[s.length() + 1];
        s.toCharArray(c, s.length() + 1);
        mqttStatus(_name, "compare", c);
    }
}


void Adc::mqtt(MqttMessage msg) {
    if (!msg.isFor(_name)) { return; }
    mqttLog("Adc/forme", msg.topic());
    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }
}

#endif