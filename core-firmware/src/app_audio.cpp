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
bool IS_CONNECTED = false;
bool IS_AMP_POWERED = false;
bool KNOB_ADJUSTS_VOLUME = true;

// ******************************
// Function Prototype Definitions
// ******************************

void radioSkip();
void onConnect();
void onDisconnect();
void ampPowerOn();
void ampPowerOff();
void ampPowerToggle();
void ampVolumeUp();
void ampVolumeDown();
void ampVolumeReset();
void sendAllStatus();
void sendAmpStatus();
void sendAmpVolume();
void sendLEDStatus();
void sendRadioStatus();
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
    led.intensity(DEFAULT_LED_INTENSITY);
    led.color("red");

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
// Connection State Callbacks
// ******************************

void onConnect() {
    led.color("green");
    IS_CONNECTED = true;
    mqttPublish("action/configure", "true");
    mqttPublish("action/connected", "true");
}

void onDisconnect() {
    led.color("red");
    IS_CONNECTED = false;
}



// ******************************
// Knob Handling
// ******************************

void checkKnob(char state, String comp) {

    if (KNOB_ADJUSTS_VOLUME) {

        switch (state) {
            case '>':
                ampVolumeUp();
                sendAmpVolume();
                mqttPublish("action/enc","up");
                break;
            case '<':
                ampVolumeDown();
                sendAmpVolume();
                mqttPublish("action/enc","down");
                break;
        }

    } else {

        switch (state) {
            case '>':
                mqttPublish("action/enc","up");
                break;
            case '<':
                mqttPublish("action/enc","down");
                break;
        }

    }

}

// ******************************
// Button Handling
// ******************************

void checkButton(char state, String comp) {

    if (IS_CONNECTED) {

        if (comp == COMPONENT_ENCBTN) {
           switch (state) {
                case 'P':
                    mqttPublish("action/button/enc","press");
                    break;
                case 'H':
                    mqttPublish("action/button/enc","hold");
                    break;
           }
        }
        if (comp == COMPONENT_LEDBTN) {
           switch (state) {
                case 'P':
                    mqttPublish("action/button/led","press");
                    break;
                case 'H':
                    mqttPublish("action/button/led","hold");
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
                    led.dim();
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

void radioSetVolume(int level) {
    radio.setVolume(level);
    RADIO_VOLUME = level;
}

void radioSetStation(int station) {
    radio.setChannel(station);
    RADIO_STATION = station;
}

void radioSkip() {
    switch (RADIO_STATION) {
        case 879:
            radioSetStation(893);
            break;
        case 893:
            radioSetStation(911);
            break;
        case 911:
            radioSetStation(937);
            break;
        case 937:
            radioSetStation(971);
            break;
        case 971:
            radioSetStation(1013);
            break;
        default:
            radioSetStation(879);
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
}

void ampPowerOff() {
    led.off();
    ampSetVolume(0);
    digitalWrite(PIN_STBY, LOW);
    IS_AMP_POWERED = false;
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
            delay(10);
        }
    } else {
        for (int i=AMP_VOLUME; i>level; i--) {
            ampVolumeDown();
            delay(10);
        }
    }
}


// ******************************
// Send Device Status
// ******************************

void sendAllStatus() {

    sendAmpStatus();
    sendLEDStatus();
    sendRadioStatus();

}

void sendAmpStatus() {

    char knob[5];
    char powered[3];

    sendAmpVolume();

    strcpy(powered,(IS_AMP_POWERED) ? "on" : "off");
    mqttPublish("status/amp/power", powered);

    strcpy(knob,(KNOB_ADJUSTS_VOLUME) ? "true" : "false");
    mqttPublish("status/knob/adjusts/volume", knob);

}

void sendAmpVolume() {
    char vol[2];
    itoa(AMP_VOLUME, vol, 10);
    mqttPublish("status/amp/vol", vol);
}

void sendRadioStatus() {

    char vol[2] = "";
    char chn[3] = "";

    itoa(RADIO_VOLUME, vol, 10);
    itoa(RADIO_STATION, chn, 10);

    mqttPublish("status/radio/vol", vol);
    mqttPublish("status/radio/station", chn);

}

void sendLEDStatus() {

    char ledi[3] = "";
    char leds[3] = "";

    strcpy(leds, (led.getState() == 1) ? "on" : "off");
    itoa(led.getIntensity(), ledi, 10);

    mqttPublish("status/led/state", leds);
    mqttPublish("status/led/intensity", ledi);
    mqttPublish("status/led/color", led.getColor());

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

    // *********************
    // CUSTOM SETUP
    // *********************

    if (equals(topic, "setup/default/radio/station")) {
        DEFAULT_RADIO_STATION = intmsg;
    }

    if (equals(topic, "setup/knob/adjusts/volume")) {
        KNOB_ADJUSTS_VOLUME = boolmsg;
    }

    if (equals(topic, "setup/led/max/intensity")) {
        led.setMaxIntensity(intmsg);
    }

    // *********************
    // REQUESTS FOR STATUS
    // *********************

    if (equals(topic, "control/status/all")) {
        sendAllStatus();
    }

    if (equals(topic, "control/configure")) {
        mqttPublish("action/configure", "true");
    }

    if (equals(topic, "control/status/amp")) {
        sendAmpStatus();
    }

    if (equals(topic, "control/status/vol")) {
        sendAmpVolume();
    }

    if (equals(topic, "control/status/led")) {
        sendLEDStatus();
    }

    if (equals(topic, "control/status/radio")) {
        sendRadioStatus();
    }

    // *********************
    // AMP POWER CONTROLS
    // *********************

    if (equals(topic, "control/power")) {

        if (equals(msg, "on")) {
            ampPowerOn();
        }

        if (equals(msg, "off")) {
            ampPowerOff();
        }

        if (equals(msg, "toggle")) {
            ampPowerToggle();
        }

    }

    // *********************
    // AMP VOLUME CONTROLS
    // *********************

    if (equals(topic, "control/volume")) {

        if (equals(msg, "up") ) {
            ampVolumeUp();
        }
        
        if (equals(msg, "down") ) {
            ampVolumeDown();
        }

        if (equals(msg, "reset") ) {
            ampVolumeReset();
        }

        if (equals(msg, "low") ) {
            ampSetVolume(AMP_VOLUME_LOW);
        }

        if (equals(msg, "med")) {
            ampSetVolume(AMP_VOLUME_MED);
        }

        if (equals(msg, "high") ) {
            ampSetVolume(AMP_VOLUME_HIGH);
        }

    }

    if (equals(topic, "control/volume/set")) {
        ampSetVolume(intmsg);
    }

    // *********************
    // RADIO CONTROLS
    // *********************

    if (equals(topic, "control/radio")) {
        if (equals(msg, "skip") ) {
            radioSkip();
        }
    }

    if (equals(topic, "control/radio/volume")) {
        radioSetVolume(intmsg);
    }

    if (equals(topic, "control/radio/station")) {
        if (equals(msg, "house")) {
            radioSetStation(DEFAULT_RADIO_STATION);
        } else {
            radioSetStation(intmsg);
        }
    }

    // *********************
    // RGB LED CONTROLS
    // *********************

    if (equals(topic, "control/led/dim")) {
        led.dim();
    }

    if (equals(topic, "control/led/color")) {
        led.color(msg);
    }

    if (equals(topic, "control/led/intensity")) {
        led.intensity(intmsg);
    }

}