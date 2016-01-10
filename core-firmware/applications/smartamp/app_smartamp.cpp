#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/lib-ds1882.h"
#include "../_inc/component-adc.h"
#include "../_inc/component-led.h"
#include "../_inc/component-relay.h"
#include "../_inc/component-button.h"
#include "../_inc/component-selector.h"
#include "../_inc/component-quadencoder.h"
#include "../_inc/lib-ir-detector.h"

// ******************************
// Definitions
// ******************************

int rssiTimer = 0;

// ******************************
// Function Prototype Definitions
// ******************************

void onConnect();
void onDisconnect();
void onKnobPress();
void onKnobHold();
void rssiToggle();
void inputToggle();
void powerToggle();
void powerOff();
void powerOn();
void potChanged();
void sendRssi();
void volChange(int i);
void volComplete(int i);
void IRUpdate();

// ******************************
// Pin Definitions
// ******************************

int pinKnob1 = A0;
int pinKnob2 = A1;
int pinBtnEnc = A2;
int pinBtnInput = D7;

int pinSelector1 = D2;
int pinSelector2 = D3;
int pinRelay12V = D4;
//int pinRelayFan = D7;
int pinRelayAmpStby = D6;

int pinAmpRSSI = A3;
int pinIR = A4;

int pinLedR = A5;
int pinLedG = A6;
int pinLedB = A7;

// ******************************
// Class instantiations
// ******************************

SimpleTimer timer;

QuadEncoder knob("knob", pinKnob1, pinKnob2, 12);

Button knobbtn("knob-btn", pinBtnEnc, INPUT_PULLUP);
Button inputbtn("input-btn", pinBtnInput, INPUT_PULLUP);

Relay rxrelay("rxrelay", pinRelay12V);
Relay ampstby("ampstby", pinRelayAmpStby);

DS1882 volume("volume");
Adc rssi("rssi", pinAmpRSSI, 12, 3.3);
LED led("led", pinLedR, pinLedG, pinLedB);
Selector source("source", pinSelector1, pinSelector2);

IRDetector IR(pinIR);

// ******************************
// Application Setup
// ******************************

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_SMARTAMP, onConnect, onDisconnect);

    led.off();
    led.color("red");
    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.intensity(DEFAULT_LED_INTENSITY);

    knob.onChange(volChange);
    knob.onComplete(volComplete);

    knobbtn.onPress(onKnobPress);
    knobbtn.onHold(onKnobHold);

    inputbtn.onPress(inputToggle);

    rssiTimer = timer.setInterval(1000, sendRssi);
    timer.disable(rssiTimer);

    volume.setup();

    attachInterrupt(pinIR, IRUpdate, FALLING);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    timer.run();
    mqttLoop();

    led.loop();
    knob.loop();
    knobbtn.loop();
    inputbtn.loop();

    while (IR.available()) {

        int code = IR.pop(); // Receive command code

        switch (code) {

            // POWER
            case 0x10EFD827:
                mqttStatus("ir", "btn", "POWER");
                powerToggle();
                break;

            // A BUTTON
            case 0x10EFF807:
                mqttStatus("ir", "btn", "A");
                source.select(1);
                source.sendStatus();
                break;

            // B BUTTON
            case 0x10EF7887:
                mqttStatus("ir", "btn", "B");
                source.select(2);
                source.sendStatus();
                break;

            // C BUTTON
            case 0x10EF58A7:
                mqttStatus("ir", "btn", "C");
                source.select(3);
                source.sendStatus();
                break;

            // UP ARROW
            case 0x10EFA05F:
                mqttStatus("ir", "btn", "UP");
                volume.up();
                volume.sendStatus();
                break;

            // DOWN ARROW
            case 0x10EF00FF:
                mqttStatus("ir", "btn", "DOWN");
                volume.down();
                volume.sendStatus();
                break;

            // LEFT ARROW
            case 0x10EF10EF:
                mqttStatus("ir", "btn", "LEFT");
                break;

            // RIGHT ARROW
            case 0x10EF807F:
                mqttStatus("ir", "btn", "RIGHT");
                break;

            // MIDDLE CIRCLE
            case 0x10EF20DF:
                mqttStatus("ir", "btn", "CIRCLE");
                break;

            default:
                //mqttStatus("ir", "code", code);
                break;
        }
    }
}

// ******************************
// Connection State Callbacks
// ******************************

void onConnect() {
    led.color("blue");
}

void onDisconnect() {
    led.color("red");
}

void onAmpPowerOn() {
    //led.on();
}

void onAmpPowerOff() {
    //led.off();
}

// ******************************
// Power State Management
// ******************************

void IRUpdate() {
    IR.update();
}

void volChange(int i) {
    volume.changeBy(i);
}

void volComplete(int i) {
    volume.sendStatus();
}

void onKnobHold() {
    rssiToggle();
}

void onKnobPress() {
    powerToggle();
}

void inputToggle() {
    switch (source.getSelection()) {
        case 1:
            source.select(3);
            break;
        case 3:
            source.select(1);
            break;
    }
    source.sendStatus();
}

void powerToggle() {
    if (rxrelay.isOpen()) {
        powerOff();
    } else {
        powerOn();
    }
}

void powerOff() {
    volume.mute();
    delay(100);
    rxrelay.close();
    delay(100);
    ampstby.close();
    led.off();
    volume.sendStatus();
}

void powerOn() {
    timer.setTimeout(3000, sendRssi);
    volume.setLevel(30);
    volume.sendStatus();
    rxrelay.open();
    ampstby.open();
    led.on();
}


// ******************************
// Manage RSSI state (sending antenna reception data)
// ******************************

void rssiToggle() {
    if (timer.isEnabled(rssiTimer)) {
        timer.disable(rssiTimer);
    } else {
        timer.enable(rssiTimer);
    }
}

void sendRssi() {
    rssi.read();
    rssi.sendStatus();
}


// ******************************
// Handle Incoming Messages
// ******************************

void mqttCustomMessageHandler(MqttMessage msg) {

    led.mqtt(msg);
    rssi.mqtt(msg);
    ampstby.mqtt(msg);
    rxrelay.mqtt(msg);
    volume.mqtt(msg);
    source.mqtt(msg);

}