#include "application.h"
#include "component-ldr.h"


#ifndef Lightspeed_h
#define Lightspeed_h

class Lightspeed
{
    public:
        Lightspeed(char* name, Ldr& _series, Ldr& _shunt, int steps);
        void mqtt(MqttMessage msg);
        void setLevel(int level);
        void sendStatus();
        char* name;

    private:
        int _level;
        int _steps;
        Ldr& series;
        Ldr& shunt;

};

Lightspeed::Lightspeed(char* _name, Ldr& _series, Ldr& _shunt, int steps) : series(_series), shunt(_shunt) {
    name = _name;
    _steps = steps - 1;
    _level = 0;
}

void Lightspeed::setLevel(int level) {

    level = (level < 0) ? 0 : level;
    level = (level > _steps) ? _steps : level;
    _level = level;

    int inverse = _steps - level;

    if (series.ready() && shunt.ready()) {

        series.setLevel(level);
        shunt.setLevel(inverse);

        delay(250);

        float v1 = series.adc.getVoltage();
        float v2 = shunt.adc.getVoltage();
        mqttStatus(name, "voltage", v1 + v2);
        sendStatus();

    } else {

        mqttAction(name, "LDRs not calibrated");

    }

}

void Lightspeed::sendStatus() {
    if (IS_CONNECTED) {
        mqttStatus(name, "level", _level);
        series.sendStatus();
        shunt.sendStatus();
    }
}

void Lightspeed::mqtt(MqttMessage msg) {

    if (!msg.isFor(name)) { return; }

    mqttLog("Lightspeed/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isCommand("level")) {
        setLevel(msg.intVal());
    }

}

#endif