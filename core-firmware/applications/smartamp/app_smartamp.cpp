#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-led.h"
#include "../_inc/component-relay.h"
#include "../_inc/component-button.h"
#include "../_inc/component-motorized-pot.h"
#include "../_inc/lib-ds1802.h"

// ******************************
// Definitions
// ******************************

int volLevel = 0;
int dacValue = 0;

// ******************************
// Function Prototype Definitions
// ******************************

void onConnect();
void onDisconnect();
void onUpBtnPress();
void onDnBtnPress();
void potChanged();
void ledReset();
void setVolume(int val);
void sendVolume();
void volUp();
void volDn();



// ******************************
// Class instantiations
// ******************************

Button upbtn("up-btn", D2, INPUT_PULLUP);
Button dnbtn("dn-btn", D4, INPUT_PULLUP);
MotorizedPot motorpot("motorpot", D3, D5, D6);
Relay rxrelay("rxrelay", D7);
Relay ampstby("ampstby", D1);

LED led("led", A1, A6, A7);
DS1802 volume(A2, D0);

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_SMARTAMP, onConnect, onDisconnect);

    led.on();
    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.intensity(DEFAULT_LED_INTENSITY);

    /* For Motorpot
    upbtn.onUp(ledReset);
    upbtn.onDown(onUpBtnPress);
    upbtn.onPress(ledReset);

    dnbtn.onUp(ledReset);
    dnbtn.onDown(onDnBtnPress);
    dnbtn.onPress(ledReset);
    */


    /* For DS1802 */
    upbtn.onDown(volUp);
    upbtn.onUp(ledReset);
    dnbtn.onDown(volDn);
    dnbtn.onUp(ledReset);


    ledReset();
    rxrelay.open();
    ampstby.open();
    setVolume(16);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    led.loop();
    upbtn.loop();
    dnbtn.loop();

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
// Button Press Handlers
// ******************************


void volUp() {
    if (volLevel < 64) {
        led.color("green");
        volLevel++;
        setVolume(volLevel);
        delay(20);
    }
}

void volDn() {
    if (volLevel > 0) {
        led.color("red");
        volLevel--;
        setVolume(volLevel);
        delay(20);
    }
}

void setVolume(int level) {
    volume.setValue(64 - level);
    volLevel = level;
    sendVolume();
}


void sendVolume() {
    mqttStatus("volume", "level", volLevel);
}


void ledReset() {
    led.color("blue");
    motorpot.stop();
    motorpot.sendStatus();
}

void onUpBtnPress() {
    led.color("green");
    motorpot.moveUp();
}

void onDnBtnPress() {
    led.color("red");
    motorpot.moveDown();
}


// ******************************
// Handle Incoming Messages
// ******************************

void mqttCustomMessageHandler(MqttMessage msg) {

    led.mqtt(msg);
    ampstby.mqtt(msg);
    rxrelay.mqtt(msg);
    motorpot.mqtt(msg);

}