#include "application.h"
#include "_core-common.h"
#include "_component-button.h"
#include "_component-quadencoder.h"

// ******************************
// Definitions
// ******************************

int PIN_ENC1 = D0;
int PIN_ENC2 = D1;
int PIN_BTN0 = D2;
int PIN_BTN1 = D3;
int PIN_BTN2 = D4;
int PIN_BTN3 = D5;
int PIN_BTN4 = D6;
int PIN_PIR  = A0;

int COMPONENT_KNOB  = 1;
int COMPONENT_POWER = 2;
int COMPONENT_BTN1  = 3;
int COMPONENT_BTN2  = 4;
int COMPONENT_BTN3  = 5;
int COMPONENT_BTN4  = 6;
int COMPONENT_PIR   = 7;

// ******************************
// Function Prototype Definitions
// ******************************


// ******************************
// Class instantiation
// ******************************

QuadEncoder knob(PIN_ENC1, PIN_ENC2);
Button button0(PIN_BTN0, INPUT_PULLDOWN);
Button button1(PIN_BTN1, INPUT_PULLDOWN);
Button button2(PIN_BTN2, INPUT_PULLDOWN);
Button button3(PIN_BTN3, INPUT_PULLDOWN);
Button button4(PIN_BTN4, INPUT_PULLDOWN);

void setup() {

    Serial.begin(9600);
    mqttSetup(DEVICE_TYPE_PANEL);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();

    defaultKnobCheck(knob.state(), COMPONENT_KNOB);
    defaultButtonCheck(button0.state(), COMPONENT_KNOB);
    defaultButtonCheck(button1.state(), COMPONENT_BTN1);
    defaultButtonCheck(button2.state(), COMPONENT_BTN2);
    defaultButtonCheck(button3.state(), COMPONENT_BTN3);
    defaultButtonCheck(button4.state(), COMPONENT_BTN4);

}
