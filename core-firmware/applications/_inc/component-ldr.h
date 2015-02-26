#include "application.h"
#include "lib-timer.h"
#include "component-dac.h"
#include "component-adc.h"
#include <math.h>

#ifndef Ldr_h
#define Ldr_h

class Ldr
{
    public:
        Ldr(char* _name, Dac& _dac, Adc& _adc, int steps);
        void mqtt(MqttMessage msg);
        void sendStatus();
        void compare();
        Dac getDac();
        Adc getAdc();
        char* name;
        Dac& dac;
        Adc& adc;

    private:
        int _steps;
        int _calibrateStep;
        int _calibrateIncr;
        int _calibrateTimer;

        float _voltageStep;


};

Ldr::Ldr(char* _name, Dac& _dac, Adc& _adc, int steps) : dac(_dac), adc(_adc) {
    name = _name;
    dac.setValue(0);
    _steps = steps;
    _voltageStep = adc.getVoltageStep();
}

void Ldr::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(name, "adc-val", adc.getValue());
        mqttStatus(name, "dac-val", dac.getValue());
    }
}

void Ldr::compare() {
    if (IS_CONNECTED) {
        int dacValue = dac.getValue();
        int adcValue = adc.getValue();
        String s = String(dacValue) + ":" + String(adcValue);
        char c[s.length() + 1];
        s.toCharArray(c, s.length() + 1);
        mqttStatus(name, "compare", c);
    }
}

void Ldr::mqtt(MqttMessage msg) {

    dac.mqtt(msg);
    adc.mqtt(msg);

    if (!msg.isFor(name)) { return; }
    mqttLog("Ldr/forme", msg.topic());
    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }
}

#endif