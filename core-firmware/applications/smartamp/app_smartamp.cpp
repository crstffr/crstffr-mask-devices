#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-led.h"
#include "../_inc/component-button.h"
#include "../_inc/component-motorized-pot.h"

// ******************************
// Definitions
// ******************************


// ******************************
// Function Prototype Definitions
// ******************************

void onConnect();
void onDisconnect();
void onUpBtnPress();
void onDnBtnPress();
void potChanged();
void ledReset();

// ******************************
// Class instantiations
// ******************************

LED led(A4, A5, A6);

Button upbtn("up-btn", D2, INPUT_PULLUP);
Button dnbtn("dn-btn", D4, INPUT_PULLUP);
MotorizedPot motorpot("motorpot", A1, D1, D0);


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

    motorpot.onChange(potChanged);

    ledReset();

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    led.loop();

    upbtn.loop();
    dnbtn.loop();
    motorpot.loop();

}

// ******************************
// Connection State Callbacks
// ******************************

void onConnect() {
    //led.color("green");
}

void onDisconnect() {
    //led.color("red");
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

void potChanged() {
    //motorpot.sendStatus();
}

void ledReset() {
    led.color("blue");
    digitalWrite(D1, LOW);
}

void onUpBtnPress() {

    led.color("green");
    digitalWrite(D0, HIGH);
    digitalWrite(D1, HIGH);
}

void onDnBtnPress() {
    led.color("red");
    digitalWrite(D0, LOW);
    digitalWrite(D1, HIGH);
}

// ******************************
// Handle Incoming Messages
// ******************************

void mqttCustomMessageHandler(char* topic, char** topicParts, int topicCount, char* msg) {

    int intmsg = atoi(msg);
    bool boolmsg = equals(msg, "true");

    char topicsize[2];
    itoa(topicCount, topicsize, 10);
    mqttLog("topic/count", topicsize);

    // ******************************
    // Incoming Data
    // ******************************

    // topicParts[1] = setup || control
    // topicParts[2] = ...
    // topicParts[3] = ...

    if (equals(topic, "setup/led/max/intensity")) {
        led.setMaxIntensity(intmsg);
        return;
    }

    // *********************
    // REQUESTS FOR STATUS
    // *********************

    if (equals(topic, "command/status/all")) {
        led.sendStatus();
        motorpot.sendStatus();
        return;
    }

    if (equals(topic, "command/status/led")) {
        led.sendStatus();
        return;
    }

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

    // *********************
    // RGB LED CONTROLS
    // *********************

    if (equals(topic, "command/led/state")) {
        if (equals(msg, "on")) {
            led.on();
        } else if (equals(msg, "off")) {
            led.off();
        }
        return;
    }

    if (equals(topic, "command/led/dim")) {
        led.dim();
        return;
    }

    if (equals(topic, "command/led/color")) {
        led.color(msg);
        return;
    }

    if (equals(topic, "command/led/intensity")) {
        led.intensity(intmsg);
        return;
    }

}