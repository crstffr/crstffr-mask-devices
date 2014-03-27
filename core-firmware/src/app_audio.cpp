#include "application.h"
#include "_common.h"
#include "_Si4703.h"
#include "_quadencoder.h"

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

String COMPONENT_LEDBTN = "1";
String COMPONENT_ENCBTN = "2";
String COMPONENT_MSCBTN = "3";
String COMPONENT_KNOB   = "4";
String COMPONENT_POWER  = "5";
String COMPONENT_VOLUME = "6";

int  AMP_VOLUME = 0;
int  RADIO_VOLUME = 0;
int  RADIO_STATION = 0;
bool IS_AUDIO_POWERED = false;

// ******************************
// Function Prototype Definitions
// ******************************

void sendStatus();
void onConnect();
void onDisconnect();
void ampPowerOn();
void ampPowerOff();
void ampPowerToggle();
void ampVolumeUp();
void ampVolumeDown();
void ampVolumeReset();
void sendAmpVolume();
void ampSetVolume(int level);
void radioSetVolume(int level);
void radioSetStation(int station);
void checkKnob(char state, String comp);
void checkButton(char state, String comp);

// ******************************
// Class instantiations
// ******************************

LED led(PIN_R, PIN_G, PIN_B);
Si4703 radio(PIN_RST, PIN_SDA, PIN_SCL);
QuadEncoder knob(PIN_ENC1, PIN_ENC2);
Button ledbtn(PIN_LEDBTN, INPUT_PULLUP);
Button encbtn(PIN_ENCBTN, INPUT_PULLUP);
Button mscbtn(PIN_MSCBTN, INPUT_PULLUP);

void setup() {

    Serial.begin(9600);
    mqttSetup(onConnect, onDisconnect);

    pinMode(PIN_POWER, OUTPUT);
    pinMode(PIN_VOLUP, OUTPUT);
    pinMode(PIN_VOLDN, OUTPUT);
    pinMode(PIN_STBY,  OUTPUT);
    pinMode(PIN_RST,   OUTPUT);

    digitalWrite(PIN_VOLUP, LOW);
    digitalWrite(PIN_VOLDN, LOW);
    digitalWrite(PIN_POWER, HIGH);

    radio.powerOn();
    radioSetVolume(DEFAULT_RADIO_VOLUME);
    radioSetStation(DEFAULT_RADIO_STATION);

    delay(500);

    digitalWrite(PIN_VOLUP, HIGH);
    digitalWrite(PIN_VOLDN, HIGH);

    led.setMaxIntensity(MAX_LED_INTENSITY);
    led.color("red");

    if (AMP_POWER_ON_BOOT) {
        ampPowerOn();
    }

}

// ******************************
// Main Loop
// ******************************

void loop() {

    mqttLoop();
    led.loop();

    checkKnob(knob.state(), COMPONENT_KNOB);
    checkButton(ledbtn.state(), COMPONENT_LEDBTN);
    checkButton(encbtn.state(), COMPONENT_ENCBTN);
    checkButton(mscbtn.state(), COMPONENT_MSCBTN);

}

// ******************************
// MQTT Connection Maintenance
// ******************************

void onConnect() {
    led.color("green");
}

void onDisconnect() {
    led.color("red");
}

void mqttCustomMessageHandler(char** topic, char* msg) {

    int intmsg = atoi(msg);

    // ******************************
    // Incoming Event Handling
    // ******************************

    // topic[0] = dev
    // topic[1] = {device id}
    // topic[2] = setup || control
    // topic[3] = whatever
    // topic[4] = ...

    if ( equals(topic[2], "setup") ) {
        if ( equals(topic[3], "default") ) {
            if ( equals(topic[4], "radio") ) {
                if ( equals(topic[5], "station") ) {
                    DEFAULT_RADIO_STATION = intmsg;
                }
            }
        }
    } else if ( equals(topic[2], "control") ) {

        mqttLog("control");

        if ( equals(topic[3], "status") ) {

            mqttLog("status");
            sendStatus();

        } else if ( equals(topic[3], "power") ) {

            mqttLog("control/power");

            // POWER CONTROLS

            if (equals(msg, "on")) {
                ampPowerOn();
            }

            if (equals(msg, "off")) {
                ampPowerOff();
            }

            if (equals(msg, "toggle")) {
                ampPowerToggle();
            }

        } else if ( equals(topic[3], "vol") ) {

            mqttLog("control/vol");

            // VOLUME CONTROLS

            if ( equals(msg, "low") ) {

                ampSetVolume(AMP_VOLUME_LOW);

            } else if ( equals(msg, "med")) {

                ampSetVolume(AMP_VOLUME_MED);

            } else if ( equals(msg, "high") ) {

                ampSetVolume(AMP_VOLUME_HIGH);

            } else if ( equals(topic[4], "set") ) {

                ampSetVolume(intmsg);

            }

        } else if (equals(topic[3], "radio")) {

            mqttLog("control/radio");

            // RADIO CONTROLS

            if (equals(topic[4], "station")) {

                mqttLog("control/radio/station");

                if (equals(msg, "house")) {
                    radioSetStation(DEFAULT_RADIO_STATION);
                } else {
                    radioSetStation(intmsg);
                }

            } else if (equals(topic[4], "vol")) {

                mqttLog("control/radio/vol");
                radioSetVolume(intmsg);

            }

        } else if (equals(topic[3], "led")) {

            mqttLog("control/led");

            // LED CONTROLS

            if (equals(topic[4], "color")) {

                mqttLog("control/led/color");
                led.color(msg);

            } else if (equals(topic[4], "intensity")) {

                mqttLog("control/led/intensity");
                led.intensity(intmsg);

            }

        }

    }

}

// ******************************
// Knob Handling
// ******************************

void checkKnob(char state, String comp) {

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

}

// ******************************
// Button Handling
// ******************************

void checkButton(char state, String comp) {
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
                led.dim();
                break;
        }
    }
    if (comp == COMPONENT_MSCBTN) {
       switch (state) {
            case 'P':
                mqttPublish("action/button/misc","press");
                break;
            case 'H':
                mqttPublish("action/button/misc","hold");
                break;
        }
    }
}

// ******************************
// Send Device Status
// ******************************

void sendStatus() {

    char vol1[2] = "";
    char vol2[2] = "";
    char chn1[3] = "";
    char ledi[3] = "";
    char leds[3] = "";

    itoa(AMP_VOLUME, vol1, 10);
    itoa(RADIO_VOLUME, vol2, 10);
    itoa(RADIO_STATION, chn1, 10);
    itoa(led.getIntensity(), ledi, 10);

    if (led.getState() == 1) {
        strcpy(leds, "on");
    } else {
        strcpy(leds, "off");
    }

    if (IS_AUDIO_POWERED) {
        mqttPublish("status/amp/power", "on");
    } else {
        mqttPublish("status/amp/power", "off");
    }

    mqttPublish("status/amp/vol", vol1);
    mqttPublish("status/radio/vol", vol2);
    mqttPublish("status/radio/station", chn1);

    mqttPublish("status/led/state", leds);
    mqttPublish("status/led/intensity", ledi);
    mqttPublish("status/led/color", led.getColor());
}

void sendAmpVolume() {
    char vol[2];
    itoa(AMP_VOLUME, vol, 10);
    mqttPublish("status/amp/vol", vol);
}

// ******************************
// Radio Commands
// ******************************

void radioSetVolume(int level) {
    radio.setVolume(level);
    RADIO_VOLUME = level;
}

void radioSetStation(int station) {
    radio.setChannel(station);
    RADIO_STATION = station;
}

// ******************************
// Amplifier Controls
// ******************************

void ampPowerOn() {
    digitalWrite(PIN_STBY, HIGH);
    IS_AUDIO_POWERED = true;
    ampSetVolume(AMP_VOLUME_DEFAULT);
    mqttPublish("status/amp/power", "on");
    led.on();
}

void ampPowerOff() {
    ampSetVolume(0);
    IS_AUDIO_POWERED = false;
    digitalWrite(PIN_STBY, LOW);
    mqttPublish("status/amp/power", "off");
    led.off();
}

void ampPowerToggle() {
    if (IS_AUDIO_POWERED) {
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
    sendAmpVolume();
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
            delay(10);
        }
    } else {
        for (int i=AMP_VOLUME; i>level; i--) {
            ampVolumeDown();
            delay(10);
        }
    }
    sendAmpVolume();
}
