#include "application.h"
#include "_core-common.h"
#include "_component-button.h"
#include "_component-quadencoder.h"
#include "_component-pir.h"

// ******************************
// Class instantiation
// ******************************

PIR pir("pir", A0);
QuadEncoder knob("knob", D0, D1);
Button btn0("btn0", D2, INPUT_PULLDOWN);
Button btn1("btn1", D3, INPUT_PULLDOWN);
Button btn2("btn2", D4, INPUT_PULLDOWN);
Button btn3("btn3", D5, INPUT_PULLDOWN);
Button btn4("btn4", D6, INPUT_PULLDOWN);

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_PANEL);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    knob.loop();
    btn0.loop();
    btn1.loop();
    btn2.loop();
    btn3.loop();
    btn4.loop();
    pir.loop();

}

void mqttCustomMessageHandler(char* topic, char** topicParts, int topicCount, char* msg) {}