#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/lib-ds1882.h"
#include "../_inc/component-adc.h"
#include "../_inc/component-led.h"
#include "../_inc/component-relay.h"
#include "../_inc/component-button.h"
#include "../_inc/component-quadencoder.h"

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
void powerToggle();
void powerOff();
void powerOn();
void potChanged();
void sendRssi();
void volUp();
void volDn();

// ******************************
// Pin Definitions
// ******************************

int pinKnob1 = A0;
int pinKnob2 = A1;
int pinBtnEnc = A2;

int pinRelay12V = D4;
int pinRelayFan = D7;
int pinRelayAmpStby = D6;

int pinLedR = A5;
int pinLedG = A6;
int pinLedB = A7;

int pinAmpRSSI = A3;

// ******************************
// Class instantiations
// ******************************

SimpleTimer timer;

QuadEncoder knob("knob", pinKnob1, pinKnob2);
Button knobbtn("knob-btn", pinBtnEnc, INPUT_PULLUP);

Relay rxrelay("rxrelay", pinRelay12V);
Relay fanrelay("fanrelay", pinRelayFan);
Relay ampstby("ampstby", pinRelayAmpStby);

LED led("led", pinLedR, pinLedG, pinLedB);
Adc rssi("rssi", pinAmpRSSI, 12, 3.3);

DS1882 volume("volume");

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

    knob.onUp(volUp);
    knob.onDown(volDn);
    knobbtn.onPress(onKnobPress);
    knobbtn.onHold(onKnobHold);

    rssiTimer = timer.setInterval(1000, sendRssi);
    timer.disable(rssiTimer);

    volume.setup();

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

void onKnobHold() {
    rssiToggle();
}

void onKnobPress() {
    powerToggle();
}

void powerToggle() {
    if (rxrelay.isOpen()) {
        powerOff();
    } else {
        powerOn();
    }
}

void powerOff() {
    led.off();
    rxrelay.close();
    ampstby.close();
    volume.setLevel(16);
}

void powerOn() {
    led.on();
    rxrelay.open();
    ampstby.open();
    volume.setLevel(16);
    timer.setTimeout(3000, sendRssi);
}

// ******************************
// Button Press Handlers
// ******************************

void volUp() {
    volume.up();
}

void volDn() {
    volume.down();
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

}