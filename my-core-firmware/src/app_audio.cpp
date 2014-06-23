#include "application.h"
#include "_core-common.h"
#include "_component-amp.h"
#include "_component-led.h"
#include "_component-radio.h"
#include "_component-button.h"
#include "_component-quadencoder.h"

// ******************************
// Definitions
// ******************************

int PIN_POWER = D5;
bool KNOB_ADJUSTS_VOLUME = true;

// ******************************
// Function Prototype Definitions
// ******************************

void onConnect();
void onDisconnect();
void onAmpPowerOn();
void onAmpPowerOff();
void onKnobUp();
void onKnobDown();
void onLedBtnPress();
void onEncBtnPress();
void onMscBtnPress();
void checkKnob();

// ******************************
// Class instantiations
// ******************************

LED led(A0, A1, A2);
Amp amp("amp", D3, 0, D6, D7);
Radio radio("radio", D2, D1, D0);
QuadEncoder knob("knob", A4, A5);
Button ledbtn("led-btn", A3, INPUT_PULLUP);
Button encbtn("knob-btn", A6, INPUT_PULLUP);
Button mscbtn("misc-btn", A7, INPUT_PULLUP);


void setup() {

    coreSetup();
    mqttSetup(DEVICE_TYPE_AUDIO, onConnect, onDisconnect);

    pinMode(PIN_POWER, OUTPUT);
    digitalWrite(PIN_POWER, HIGH);

    radio.powerOn();
    radio.setVolume(DEFAULT_RADIO_VOLUME);
    radio.setStation(DEFAULT_RADIO_STATION);

    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.intensity(DEFAULT_LED_INTENSITY);
    led.color("red");

    ledbtn.onPress(onLedBtnPress);
    encbtn.onPress(onEncBtnPress);
    mscbtn.onPress(onMscBtnPress);

    amp.onPowerOn(onAmpPowerOn);
    amp.onPowerOff(onAmpPowerOff);

    knob.onUp(onKnobUp);
    knob.onDown(onKnobDown);

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    led.loop();

    knob.loop();
    ledbtn.loop();
    encbtn.loop();
    mscbtn.loop();

}

// ******************************
// Connection State Callbacks
// ******************************

void onConnect() {
    led.color("green");
}

void onDisconnect() {
    led.color("red");
}

void onAmpPowerOn() {
    led.on();
    //led.sendStatus();
}

void onAmpPowerOff() {
    led.off();
    //led.sendStatus();
}


// ******************************
// Default Buttons When Not Connected
// ******************************

void onEncBtnPress() {
    amp.powerToggle();
}

void onMscBtnPress() {
    if (!IS_CONNECTED) {
        radio.skipStation();
    }
}

void onLedBtnPress() {
    if (!IS_CONNECTED) {
        mqttConnect();
    }
}


// ******************************
// Knob Handling
// ******************************

void onKnobUp() {
    amp.volumeUp();
    amp.sendVolume();
}

void onKnobDown() {
    amp.volumeDown();
    amp.sendVolume();
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

    if (equals(topic, "setup/default/radio/station")) {
        DEFAULT_RADIO_STATION = intmsg;
        return;
    }

    if (equals(topic, "setup/knob/adjusts/volume")) {
        KNOB_ADJUSTS_VOLUME = boolmsg;
        return;
    }

    if (equals(topic, "setup/led/max/intensity")) {
        led.setMaxIntensity(intmsg);
        return;
    }

    // *********************
    // REQUESTS FOR STATUS
    // *********************

    if (equals(topic, "command/status/all")) {
        amp.sendStatus();
        led.sendStatus();
        radio.sendStatus();
        return;
    }

    if (equals(topic, "command/status/amp")) {
        amp.sendStatus();
        return;
    }

    if (equals(topic, "command/status/vol")) {
        amp.sendVolume();
        return;
    }

    if (equals(topic, "command/status/led")) {
        led.sendStatus();
        return;
    }

    if (equals(topic, "command/status/radio")) {
        radio.sendStatus();
        return;
    }

    // *********************
    // AMP POWER CONTROLS
    // *********************

    if (equals(topic, "command/power/state")) {

        if (equals(msg, "on")) {
            amp.powerOn();
            return;
        }

        if (equals(msg, "off")) {
            amp.powerOff();
            return;
        }

        if (equals(msg, "toggle")) {
            amp.powerToggle();
            return;
        }

        return;

    }

    // *********************
    // AMP VOLUME CONTROLS
    // *********************

    if (equals(topic, "command/volume/change")) {

        if (equals(msg, "up") ) {
            amp.volumeUp();
            return;
        }
        
        if (equals(msg, "down") ) {
            amp.volumeDown();
            return;
        }

        if (equals(msg, "reset") ) {
            amp.volumeReset();
            return;
        }

        if (equals(msg, "low") ) {
            amp.volumeSet(AMP_VOLUME_LOW);
            return;
        }

        if (equals(msg, "med")) {
            amp.volumeSet(AMP_VOLUME_MED);
            return;
        }

        if (equals(msg, "high") ) {
            amp.volumeSet(AMP_VOLUME_HIGH);
            return;
        }

        return;

    }

    if (equals(topic, "command/volume/set")) {
        amp.volumeSet(intmsg);
        return;
    }

    // *********************
    // RADIO CONTROLS
    // *********************

    if (equals(topic, "command/radio/volume")) {
        radio.setVolume(intmsg);
        return;
    }

    if (equals(topic, "command/radio/station")) {
        if (equals(msg, "skip") ) {
            radio.skipStation();
            return;
        }

        if (equals(msg, "house")) {
            radio.setStation(DEFAULT_RADIO_STATION);
            return;
        } else {
            radio.setStation(intmsg);
            return;
        }

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