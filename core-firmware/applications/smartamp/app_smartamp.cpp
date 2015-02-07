#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-led.h"
#include "../_inc/component-relay.h"
#include "../_inc/component-button.h"
#include "../_inc/component-motorized-pot.h"
//#include "../_inc/lib-ds1802.h"

// ******************************
// Definitions
// ******************************

int volLevel = 0;

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

// ******************************
// Class instantiations
// ******************************

LED led("led", A1, A6, A7);

Button upbtn("up-btn", D2, INPUT_PULLUP);
Button dnbtn("dn-btn", D4, INPUT_PULLUP);

MotorizedPot motorpot("motorpot", A2, D1, D0);

Relay rxrelay("rxrelay", D7);

void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_SMARTAMP, onConnect, onDisconnect);

    led.on();
    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.intensity(DEFAULT_LED_INTENSITY);

    upbtn.onUp(ledReset);
    upbtn.onDown(onUpBtnPress);
    upbtn.onPress(ledReset);

    dnbtn.onUp(ledReset);
    dnbtn.onDown(onDnBtnPress);
    dnbtn.onPress(ledReset);

    ledReset();

    rxrelay.open();

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

void ledReset() {
    led.color("blue");
    digitalWrite(D1, LOW);
    motorpot.sendStatus();
}

void onUpBtnPress() {

    led.color("green");

    //volLevel++;
    //setVolume(volLevel);
    //delay(100);

    digitalWrite(D0, HIGH);
    digitalWrite(D1, HIGH);
}

void onDnBtnPress() {
    led.color("red");

    //volLevel--;
    //setVolume(volLevel);
    //delay(100);

    digitalWrite(D0, LOW);
    digitalWrite(D1, HIGH);
}


// ******************************
// Handle Incoming Messages
// ******************************

void mqttCustomMessageHandler(MqttMessage msg) {

    led.mqtt(msg);
    rxrelay.mqtt(msg);
    motorpot.mqtt(msg);


    /*

    // *********************
    // AMP POWER CONTROLS
    // *********************

    if (equals(topic, "command/amp/power")) {

        if (equals(msg, "on")) {
            // amp.powerOn();
            return;
        }

        if (equals(msg, "off")) {
            // amp.powerOff();
            return;
        }

        if (equals(msg, "toggle")) {
            // amp.powerToggle();
            return;
        }

        return;

    }

    // *********************
    // AMP VOLUME CONTROLS
    // *********************

    if (equals(topic, "command/amp/volume")) {

        if (equals(msg, "up") ) {
            //amp.volumeUp();
            //amp.sendVolume();
            return;
        }

        if (equals(msg, "down") ) {
            //amp.volumeDown();
            //amp.sendVolume();
            return;
        }

        if (equals(msg, "reset") ) {
            //amp.volumeReset();
            //amp.sendVolume();
            return;
        }

        if (equals(msg, "low") ) {
            //amp.volumeSet(AMP_VOLUME_LOW);
            //amp.sendVolume();
            return;
        }

        if (equals(msg, "med")) {
            //amp.volumeSet(AMP_VOLUME_MED);
            //amp.sendVolume();
            return;
        }

        if (equals(msg, "high") ) {
            //amp.volumeSet(AMP_VOLUME_HIGH);
            //amp.sendVolume();
            return;
        }

        if (intmsg >= 0 && intmsg <= 64) {
            //amp.volumeSet(intmsg);
            return;
        }

        return;

    }

    */



}