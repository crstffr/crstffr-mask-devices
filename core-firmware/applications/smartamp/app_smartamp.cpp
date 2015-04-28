#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/lib-ds1882.h"
#include "../_inc/component-adc.h"
#include "../_inc/component-led.h"
#include "../_inc/component-relay.h"
#include "../_inc/component-button.h"
#include "../_inc/component-quadencoder.h"
//#include "../_inc/component-rotaryencoder.h"

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
void volChange(int i);
void volComplete(int i);

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

QuadEncoder knob("knob", pinKnob1, pinKnob2, 12);
//RotaryEncoder encoder("encoder", pinKnob1, pinKnob2, 12);

Button knobbtn("knob-btn", pinBtnEnc, INPUT_PULLUP);

Relay rxrelay("rxrelay", pinRelay12V);
Relay fanrelay("fanrelay", pinRelayFan);
Relay ampstby("ampstby", pinRelayAmpStby);

DS1882 volume("volume");
Adc rssi("rssi", pinAmpRSSI, 12, 3.3);
LED led("led", pinLedR, pinLedG, pinLedB);




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

    //knob.onUp(volUp);
    //knob.onDown(volDn);
    knob.onChange(volChange);
    knob.onComplete(volComplete);

    knobbtn.onPress(onKnobPress);
    knobbtn.onHold(onKnobHold);

    //encoder.onUp(volUp);
    //encoder.onDown(volDn);
    //encoder.onChange(onEncoderChange);
    //encoder.onComplete(onEncoderComplete);

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

    //encoder.loop();

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

void volChange(int i) {
    volume.changeBy(i);
}

void volComplete(int i) {
    //mqttStatus("volume", "changed", i);
    volume.sendStatus();
}

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
    volume.mute();
    delay(100);
    rxrelay.close();
    delay(100);
    ampstby.close();
    led.off();
}

void powerOn() {
    timer.setTimeout(3000, sendRssi);
    volume.setLevel(30);
    rxrelay.open();
    ampstby.open();
    led.on();
}

// ******************************
// Button Press Handlers
// ******************************

void volUp() {
    volume.up();
    volume.sendStatus();
}

void volDn() {
    volume.down();
    volume.sendStatus();
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