#include "application.h"
#include <math.h>

#ifndef Adc_h
#define Adc_h

class Adc
{
    public:
        Adc(char* name, int pin, int bitDepth, float refVoltage);
        void compare(int value);
        void mqtt(MqttMessage msg);
        float getVoltageStep();
        float getVoltage();
        void sendVoltage();
        void sendStatus();
        void sendValue();
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
};

Adc::Adc(char* name, int pin, int bitDepth, float refVoltage) {
    _pin = pin;
    _value = 0;
    _voltage = 0;
    _name = name;
    _bitDepth = bitDepth;
    _refVoltage = refVoltage;
    _resolution = pow(2.0, bitDepth) - 1;
    _voltageStep = _refVoltage / _resolution;
}

void Adc::read() {
    _value = analogRead(_pin);
    _voltage = _value * _voltageStep;
}

int Adc::getValue() {
    read();
    return _value;
}

float Adc::getVoltage() {
    read();
    return _voltage;
}

float Adc::getVoltageStep() {
    return _voltageStep;
}

void Adc::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "value", _value);
        mqttStatus(_name, "voltage", _voltage);
    }
}

void Adc::sendValue() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "value", _value);
    }
}

void Adc::sendVoltage() {
    if (IS_CONNECTED) {
        mqttStatus(_name, "voltage", _voltage);
    }
}

void Adc::compare(int value) {
    if (IS_CONNECTED) {
        String s = String(value) + ":" + String(_value);
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