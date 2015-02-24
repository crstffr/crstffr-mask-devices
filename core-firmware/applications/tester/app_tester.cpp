#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-dac.h"
#include "../_inc/component-adc.h"

// ******************************
// Definitions
// ******************************



// ******************************
// Function Prototype Definitions
// ******************************

void compare1();
void compare2();
void calibrate1();

// ******************************
// Class instantiation
// ******************************

Dac dac1("dac1", 0x63);
Adc adc1("adc1", A1, 12, 3.3);

Dac dac2("dac2", 0x62);
Adc adc2("adc2", A2, 12, 3.3);

void setup() {

    coreSetup();
    mqttSetup("tester");

    //dac1.onChange(compare1);
    //dac2.onChange(compare2);

}

// ******************************
// Main Loop
// ******************************

void loop() {
    mqttLoop();
}

void calibrate1() {

    int i = 0;
    int step = 1;
    int steps = 32;
    float vRef = 3.3;
    int resolution = 4095;
    float vStep = vRef / steps;
    float goalV = step * vStep;
    float currV = 0.0;
    float prevV = 0.0;

    mqttStatus("adc1", "goal", goalV);

    for (i = 0; i < resolution; i++) {

        dac1.setValue(i);

        delay(110);

        currV = adc1.getVoltage();

        mqttStatus("dac1", "value", i);
        mqttStatus("adc1", "currV", currV);

        if (currV >= goalV) {

            step = step + 1;
            goalV = step * vStep;

            mqttStatus("adc1", "step", step);
            mqttStatus("adc1", "goalV", goalV);
            adc1.compare(i);

            if (step >= steps) {
                continue;
            }
        }
    }
}

void compare1() {
    delay(100);
    adc1.read();
    adc1.compare(dac1.getValue());
}

void compare2() {
    delay(100);
    adc1.read();
    adc2.compare(dac2.getValue());
}

void mqttCustomMessageHandler(MqttMessage msg) {

    dac1.mqtt(msg);
    adc1.mqtt(msg);

    dac2.mqtt(msg);
    adc2.mqtt(msg);

    if (msg.is("calibrate")) {
        calibrate1();
    }

}