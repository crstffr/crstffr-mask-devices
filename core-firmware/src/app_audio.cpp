#include "application.h"
#include "_core-common.h"
#include "_component-led.h"
#include "_component-radio.h"
#include "_component-button.h"
#include "_component-quadencoder.h"

// ******************************
// Definitions
// ******************************

int PIN_SCL   = D0;
int PIN_SDA   = D1;
int PIN_RST   = D2;
int PIN_STBY  = D3;
int PIN_POWER = D5;
int PIN_VOLUP = D6;
int PIN_VOLDN = D7;

int PIN_R = A0;
int PIN_G = A1;
int PIN_B = A2;
int PIN_ENC1   = A4;
int PIN_ENC2   = A5;
int PIN_LEDBTN = A3;
int PIN_ENCBTN = A6;
int PIN_MSCBTN = A7;

int COMPONENT_KNOB   = 1;
int COMPONENT_POWER  = 2;
int COMPONENT_VOLUME = 3;
int COMPONENT_LEDBTN = 4;
int COMPONENT_ENCBTN = 5;
int COMPONENT_MSCBTN = 6;

int  AMP_VOLUME = 0;
int  RADIO_VOLUME = 0;
int  RADIO_STATION = 0;
bool IS_AMP_POWERED = false;
bool KNOB_ADJUSTS_VOLUME = true;

// ******************************
// Function Prototype Definitions
// ******************************

void onConnect();
void onDisconnect();
void radioSkip();
void ampPowerOn();
void ampPowerOff();
void ampPowerToggle();
void ampVolumeUp();
void ampVolumeDown();
void ampVolumeReset();
void sendAllStatus();
void sendAmpStatus();
void sendPowerStatus();
void sendAmpVolume();
void ampSetVolume(int level);
void knobCheck(char state, int comp);
void checkButton(char state, int comp);

// ******************************
// Class instantiations
// ******************************

LED led(PIN_R, PIN_G, PIN_B);
Radio radio(PIN_RST, PIN_SDA, PIN_SCL);
QuadEncoder knob(PIN_ENC1, PIN_ENC2);
Button ledbtn(PIN_LEDBTN, INPUT_PULLUP);
Button encbtn(PIN_ENCBTN, INPUT_PULLUP);
Button mscbtn(PIN_MSCBTN, INPUT_PULLUP);

void setup() {

    Serial.begin(9600);
    mqttSetup(DEVICE_TYPE_AUDIO, onConnect, onDisconnect);

    pinMode(PIN_POWER, OUTPUT);
    pinMode(PIN_VOLUP, OUTPUT);
    pinMode(PIN_VOLDN, OUTPUT);
    pinMode(PIN_STBY,  OUTPUT);
    pinMode(PIN_RST,   OUTPUT);

    digitalWrite(PIN_VOLUP, LOW);
    digitalWrite(PIN_VOLDN, LOW);
    digitalWrite(PIN_POWER, HIGH);

    radio.powerOn();
    radio.setVolume(DEFAULT_RADIO_VOLUME);
    radio.setStation(DEFAULT_RADIO_STATION);

    delay(500);

    digitalWrite(PIN_VOLUP, HIGH);
    digitalWrite(PIN_VOLDN, HIGH);

    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.intensity(DEFAULT_LED_INTENSITY);
    led.color("red");

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    led.loop();

    knobCheck(knob.state(), COMPONENT_KNOB);
    checkButton(ledbtn.state(), COMPONENT_LEDBTN);
    checkButton(encbtn.state(), COMPONENT_ENCBTN);
    checkButton(mscbtn.state(), COMPONENT_MSCBTN);

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

// ******************************
// Knob Handling
// ******************************

void knobCheck(char state, int comp) {

    if (KNOB_ADJUSTS_VOLUME) {

        switch (state) {
            case '>':
                ampVolumeUp();
                sendAmpVolume();
                break;
            case '<':
                ampVolumeDown();
                sendAmpVolume();
                break;
        }

    } else {

        defaultKnobCheck(state, comp);

    }

}

// ******************************
// Button Handling
// ******************************

void checkButton(char state, int comp) {

    if (IS_CONNECTED) {

        if (comp == COMPONENT_ENCBTN) {
           switch (state) {
                case 'P':
                    mqttPublish("action/enc-button","press");
                    break;
                case 'H':
                    mqttPublish("action/enc-button","hold");
                    break;
           }
        }
        if (comp == COMPONENT_LEDBTN) {
           switch (state) {
                case 'P':
                    mqttPublish("action/led-button","press");
                    break;
                case 'H':
                    mqttPublish("action/led-button","hold");
                    break;
            }
        }
        if (comp == COMPONENT_MSCBTN) {
           switch (state) {
                case 'P':
                    mqttPublish("action/misc-button","press");
                    break;
                case 'H':
                    mqttPublish("action/misc-button","hold");
                    break;
            }
        }

    } else {

        if (comp == COMPONENT_ENCBTN) {
           switch (state) {
                case 'P':
                    ampPowerToggle();
                    break;
           }
        }
        if (comp == COMPONENT_LEDBTN) {
           switch (state) {
                case 'P':
                    mqttConnect();
                    break;
            }
        }
        if (comp == COMPONENT_MSCBTN) {
           switch (state) {
                case 'P':
                    radioSkip();
                    break;
            }
        }

    }


}

// ******************************
// Radio Commands
// ******************************

void radioSkip() {
    switch (RADIO_STATION) {
        case 879:
            radio.setStation(893);
            break;
        case 893:
            radio.setStation(911);
            break;
        case 911:
            radio.setStation(937);
            break;
        case 937:
            radio.setStation(971);
            break;
        case 971:
            radio.setStation(1013);
            break;
        default:
            radio.setStation(879);
            break;
    }
}

// ******************************
// Amplifier Commands
// ******************************

void ampPowerOn() {
    led.on();
    digitalWrite(PIN_STBY, HIGH);
    ampSetVolume(AMP_VOLUME_DEFAULT);
    IS_AMP_POWERED = true;
    sendAmpStatus();
    led.sendStatus();
}

void ampPowerOff() {
    led.off();
    ampSetVolume(0);
    digitalWrite(PIN_STBY, LOW);
    IS_AMP_POWERED = false;
    sendAmpStatus();
    led.sendStatus();
}

void ampPowerToggle() {
    if (IS_AMP_POWERED) {
        ampPowerOff();
    } else {
        ampPowerOn();
    }
}

void ampVolumeReset() {
    for (int i=0; i<=100; i++) {
        ampVolumeDown();
        delay(10);
    }
    AMP_VOLUME = 0;
}

void ampVolumeUp() {
    delay(5);
    digitalWrite(PIN_VOLUP, LOW);
    delay(5);
    digitalWrite(PIN_VOLUP, HIGH);
    if (AMP_VOLUME < 64) {
        AMP_VOLUME++;
    }
}

void ampVolumeDown() {
    delay(5);
    digitalWrite(PIN_VOLDN, LOW);
    delay(5);
    digitalWrite(PIN_VOLDN, HIGH);
    if (AMP_VOLUME > 0) {
        AMP_VOLUME--;
    }
}

void ampSetVolume(int level) {
    if (level > AMP_VOLUME) {
        for (int i=AMP_VOLUME; i<level; i++) {
            ampVolumeUp();
            //delay(10);
        }
    } else {
        for (int i=AMP_VOLUME; i>level; i--) {
            ampVolumeDown();
            //delay(10);
        }
    }
}


// ******************************
// Send Device Status
// ******************************

void sendAllStatus() {

    char knob[5];

    sendAmpStatus();

    strcpy(knob,(KNOB_ADJUSTS_VOLUME) ? "true" : "false");
    mqttPublish("status/enc/adjusts-volume", knob);

    led.sendStatus();
    radio.sendStatus();

}

void sendPowerStatus() {
    char powered[3];
    strcpy(powered,(IS_AMP_POWERED) ? "on" : "off");
    mqttPublish("status/power/state", powered);
}

void sendAmpStatus() {
    sendPowerStatus();
    sendAmpVolume();
}

void sendAmpVolume() {
    char vol[2];
    itoa(AMP_VOLUME, vol, 10);
    mqttPublish("status/volume/level", vol);
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
        sendAllStatus();
        return;
    }

    if (equals(topic, "command/status/amp")) {
        sendAmpStatus();
        return;
    }

    if (equals(topic, "command/status/vol")) {
        sendAmpVolume();
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

        if (equals(msg, "on") && !IS_AMP_POWERED) {
            ampPowerOn();
            return;
        }

        if (equals(msg, "off") && IS_AMP_POWERED) {
            ampPowerOff();
            return;
        }

        if (equals(msg, "toggle")) {
            ampPowerToggle();
            return;
        }

        return;

    }

    // *********************
    // AMP VOLUME CONTROLS
    // *********************

    if (equals(topic, "command/volume/change")) {

        if (equals(msg, "up") ) {
            ampVolumeUp();
            return;
        }
        
        if (equals(msg, "down") ) {
            ampVolumeDown();
            return;
        }

        if (equals(msg, "reset") ) {
            ampVolumeReset();
            return;
        }

        if (equals(msg, "low") ) {
            ampSetVolume(AMP_VOLUME_LOW);
            return;
        }

        if (equals(msg, "med")) {
            ampSetVolume(AMP_VOLUME_MED);
            return;
        }

        if (equals(msg, "high") ) {
            ampSetVolume(AMP_VOLUME_HIGH);
            return;
        }

        return;

    }

    if (equals(topic, "command/volume/set")) {
        ampSetVolume(intmsg);
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
            radioSkip();
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