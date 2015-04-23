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
        void calibrate(int step, int dacValue);
        void compare(int dacValue);
        void setLevel(int level);
        void sendCalibration();
        void calibrated();
        void sendStatus();
        void compare();
        void reset();
        bool ready();
        char* name;
        Dac& dac;
        Adc& adc;

    private:
        int _steps;
        bool _calibrated;
        int _calibration[32];

};

Ldr::Ldr(char* _name, Dac& _dac, Adc& _adc, int steps) : dac(_dac), adc(_adc) {
    name = _name;
    _steps = steps - 1;
    _calibrated = false;
    reset();
}

void Ldr::reset() {
    dac.setValue(0);
    delay(250);
    sendStatus();
}

void Ldr::setLevel(int level) {
    level = (level < 0) ? 0 : level;
    level = (level > _steps) ? _steps : level;

    if (_calibrated) {
        dac.setValue(_calibration[level]);
        mqttStatus(name, "step", level);
    } else {
        mqttAction(name, "not calibrated");
    }
}

void Ldr::calibrate(int step, int dacValue) {
    _calibration[step] = dacValue;
}

void Ldr::calibrated() {
    _calibrated = true;
    mqttStatus(name, "calibrated", "true");
}

bool Ldr::ready() {
    return _calibrated;
}

void Ldr::sendCalibration() {

/*
    String s = "";
    int size1 = sizeof(_calibration);
    int size2 = size1/sizeof(int);

    mqttStatus(name, "calibrationSize", size2);

    for (int i = 1; i < size2; i++) {
        //mqttStatus(name, "calibrate", _calibration[i]);
        //delay(500);
        String si = String(_calibration[i]);
        s = s + si + ",";
        mqttStatus(name, "calibrate", s);
        delay(100);
    }

    mqttStatus(name, "calibrate", "done");
    //char c[s.length() + 1];
    //s.toCharArray(c, s.length() + 1);
    //mqttStatus(name, "calibration", c);
*/

}

void Ldr::compare(int dacValue) {
    if (IS_CONNECTED) {
        int adcValue = adc.getValue();
        String s = String(dacValue) + ":" + String(adcValue);
        char c[s.length() + 1];
        s.toCharArray(c, s.length() + 1);
        mqttStatus(name, "compare", c);
    }
}

void Ldr::compare() {
    int dacValue = dac.getValue();
    compare(dacValue);
}


void Ldr::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(name, "dac-val", dac.getValue());
        mqttStatus(name, "adc-val", adc.getValue());
        mqttStatus(name, "adc-vol", adc.getVoltage());
        mqttStatus(name, "calibrated", (_calibrated) ? "true" : "false");
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