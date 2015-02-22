#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-button.h"
#include "../_inc/component-dac.h"

// ******************************
// Definitions
// ******************************

int feedbackPin = A1;

// ******************************
// Function Prototype Definitions
// ******************************

void runTest();
void readADC();
void startTest();
void stopTest();

// ******************************
// Class instantiation
// ******************************

Button btn1("btn1", D3, INPUT_PULLUP);
Button btn2("btn2", D4, INPUT_PULLUP);
Dac dac("dac", 0x62);

SimpleTimer testTimer;
int testTimerNum;

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_TESTER);

    dac.onChange(readADC);

    testTimerNum = testTimer.setInterval(100, runTest);
    testTimer.disable(testTimerNum);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    btn1.loop();
    btn2.loop();
    testTimer.run();

}

void runTest() {
    int newValue = dac.getValue() + 10;
    dac.setValue(newValue);

    if (newValue > 4095) {
        stopTest();
    }
}

void readADC() {

    delay(200);

    String s;
    int dacValue = dac.getValue();
    int adcValue = analogRead(feedbackPin);

    s = String(dacValue) + ":" + String(adcValue);
    char c[s.length() + 1];
    s.toCharArray(c, s.length() + 1);
    mqttStatus("adc", "value", c);
}

void startTest() {
    testTimer.enable(testTimerNum);
}

void stopTest() {
    testTimer.disable(testTimerNum);
}

void mqttCustomMessageHandler(MqttMessage msg) {

    dac.mqtt(msg);

    if (msg.isFor("test")) {

        if (msg.is("start")) {
            startTest();
            mqttStatus("test", "state", "running");
        }

        if (msg.is("stop")) {
            stopTest();
            mqttStatus("test", "state", "stopped");
        }

    }

}