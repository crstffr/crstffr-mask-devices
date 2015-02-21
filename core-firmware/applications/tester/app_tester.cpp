#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-button.h"
#include "../_inc/component-dac.h"

// ******************************
// Definitions
// ******************************

int dacValue = 0;
int feedbackPin = A1;
bool testing = false;

// ******************************
// Function Prototype Definitions
// ******************************

void onBtnPress();
void measureDAC();
void stopMeasuringDAC();
void startMeasuringDAC();


// ******************************
// Class instantiation
// ******************************

Button btn1("btn1", D3, INPUT_PULLUP);
Button btn2("btn2", D4, INPUT_PULLUP);
Dac dac("dac", 0x62);

SimpleTimer dacTimer;
int dacTimerNum;

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_TESTER);

    dac.begin();
    dac.setValue(0, true);
    dacTimerNum = dacTimer.setInterval(100, measureDAC);
    dacTimer.disable(dacTimerNum);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    btn1.loop();
    btn2.loop();
    dacTimer.run();

}

void measureDAC() {

    String s;
    int adcValue;
    int dacIncrement = 10;

    dac.setValue(dacValue);
    dac.sendStatus();

    adcValue = analogRead(feedbackPin);

    s = String(dacValue) + ":" + String(adcValue);
    char c[s.length() + 1];
    s.toCharArray(c, s.length() + 1);
    mqttStatus("adc", "value", c);

    dacValue = dacValue + dacIncrement;

    if (dacValue > 4095) {
        stopMeasuringDAC();
        dacValue = 0;
    }
}

void startMeasuringDAC() {
    dacTimer.enable(dacTimerNum);
    testing = true;
}

void stopMeasuringDAC() {
    dacTimer.disable(dacTimerNum);
    testing = false;
}

void mqttCustomMessageHandler(MqttMessage msg) {

    dac.mqtt(msg);

    if (msg.isFor("test")) {

        if (msg.is("start")) {
            startMeasuringDAC();
            mqttStatus("test", "state", "running");
        }

        if (msg.is("stop")) {
            stopMeasuringDAC();
            mqttStatus("test", "state", "stopped");
        }

    }

}