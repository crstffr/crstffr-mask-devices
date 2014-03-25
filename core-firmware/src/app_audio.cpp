#include "application.h"
#include "_quadencoder.h"
#include "_Si4703.h"
#include "_common.h"

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

int PIN_ENC1   = A4;
int PIN_ENC2   = A5;
int PIN_ENCBTN = A6;
int PIN_MSCBTN = A7;
int PIN_LEDBTN = A3;
int RGB_LED[3] = {A0, A1, A2};

String COMPONENT_LEDBTN = "1";
String COMPONENT_ENCBTN = "2";
String COMPONENT_MSCBTN = "3";
String COMPONENT_KNOB   = "4";
String COMPONENT_POWER  = "5";
String COMPONENT_VOLUME = "6";

int  VOL_LEVEL = 0;
char DEVICE_ID[25];
bool IS_AUDIO_POWERED = false;
bool IS_CONNECTED_TO_SERVER = false;

// ******************************
// Function Prototype Definitions
// ******************************

void status();
void audioPowerOn();
void audioPowerOff();
void audioTogglePower();
void audioVolumeUp();
void audioVolumeDown();
void audioVolumeReset();
void sendAudioVolumeLevel();
void audioVolumeSet(int level);

void checkKnob(char state, String comp);
void checkButton(char state, String comp);

void mqttConnect();
void mqttSubscribe(char* topic);
void mqttPublish(char* topic, char* payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttMessageHandler(String tStr, char* tChar, String pStr, char* pChar);

// ******************************
// Class instantiation
// ******************************

TCPClient tcp;
SimpleTimer timer;
int connectTimer;

PubSubClient mqtt(MQTT_HOST, 1883, mqttCallback, tcp);
LED led(RGB_LED[0], RGB_LED[1], RGB_LED[2]);
Si4703 radio(PIN_RST, PIN_SDA, PIN_SCL);
QuadEncoder knob(PIN_ENC1, PIN_ENC2);
Button ledbtn(PIN_LEDBTN, INPUT_PULLUP);
Button encbtn(PIN_ENCBTN, INPUT_PULLUP);
Button mscbtn(PIN_MSCBTN, INPUT_PULLUP);

void setup()
{

    String deviceID = Spark.deviceID();
    deviceID.toCharArray(DEVICE_ID, 25);

    mqttConnect();
    connectTimer = timer.setInterval(1000, mqttConnect);

    pinMode(PIN_POWER, OUTPUT);
    pinMode(PIN_VOLUP, OUTPUT);
    pinMode(PIN_VOLDN, OUTPUT);
    pinMode(PIN_STBY,  OUTPUT);
    pinMode(PIN_RST,   OUTPUT);

    digitalWrite(PIN_VOLUP, LOW);
    digitalWrite(PIN_VOLDN, LOW);
    digitalWrite(PIN_POWER, HIGH);

    radio.powerOn();
    radio.setVolume(15);
    radio.setChannel(879);

    delay(500);

    digitalWrite(PIN_VOLUP, HIGH);
    digitalWrite(PIN_VOLDN, HIGH);

    led.setMaxIntensity(MAX_LED_INTENSITY);
    audioPowerOn();

}

// ******************************
// Main Loop
// ******************************

void loop()
{
    led.loop();
    mqtt.loop();
    timer.run();

    checkKnob(knob.state(), COMPONENT_KNOB);
    checkButton(ledbtn.state(), COMPONENT_LEDBTN);
    checkButton(encbtn.state(), COMPONENT_ENCBTN);
    checkButton(mscbtn.state(), COMPONENT_MSCBTN);

    if (mqtt.connected()) {
        timer.disable(connectTimer);
    } else {
        timer.enable(connectTimer);
    }

}

// ******************************
// MQTT Connection Maintenance
// ******************************

void mqttConnect() {
    if (!mqtt.connected()) {
        if (mqtt.connect(DEVICE_ID)) {
            mqttSubscribe("setup/#");
            mqttSubscribe("control/#");
            led.color("green");
        } else {
            led.color("red");
        }
    }
}

void mqttSubscribe(char* topic) {

    // This makes it so that every event that is subscribed
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String devTopic = "dev/" + Spark.deviceID() + "/";
    String newTopic = String(topic);
    String fullTopic = devTopic + newTopic;
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);

    mqtt.subscribe(charTopic, 1);
}

void mqttPublish(char* topic, char* payload) {

    // This makes it so that every event that is published
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String devTopic = "dev/" + Spark.deviceID() + "/";
    String newTopic = String(topic);
    String fullTopic = devTopic + newTopic;
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);

    mqtt.publish(charTopic, payload);

}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

    char msg[length];
    String ps = String((const char*) payload);
    ps.toCharArray(msg, length + 1);

    String ts = String(topic);
    String tps = ts.substring(29, ts.length() + 1);
    char tpc[ts.length() + 1];
    tps.toCharArray(tpc, ts.length() + 1);

    mqttPublish("log/callback/topic", tpc);
    mqttPublish("log/callback/message", msg);

    // ******************************
    // Incoming Event Handling
    // ******************************


    if (tps == "control/power") {
        mqttPublish("log/callback/power", "matched");

        if (strcmp(msg, "on") == 0) {
            audioPowerOn();
            mqttPublish("log/callback/power/on", "matched");
        }

        if (strcmp(msg, "off") == 0) {
            mqttPublish("log/callback/power/off", "matched");
            audioPowerOff();
        }
    }

    if (tps == "control/vol") {
        mqttPublish("log/callback/vol", "matched");

        if (strcmp(msg, "low") == 0) {
            audioVolumeSet(VOL_LEVEL_LOW);

        } else if (strcmp(msg, "med") == 0) {
            audioVolumeSet(VOL_LEVEL_MED);

        } else if (strcmp(msg, "high") == 0) {
            audioVolumeSet(VOL_LEVEL_HIGH);

        } else {
            int level = ps.toInt();
            audioVolumeSet(level);

        }
    }

    if (tps == "control/radio/station") {
        mqttPublish("log/callback/radio/station", "matched");
        int station = ps.toInt();
        radio.setChannel(station);
    }

    if (tps == "control/radio/vol") {
        mqttPublish("log/callback/radio/vol", "matched");
        int level = ps.toInt();
        radio.setVolume(level);
    }

    if (tps == "control/led") {
        mqttPublish("log/callback/led", "matched");
        led.color(msg);
    }


}

// ******************************
// Knob Handling
// ******************************

void checkKnob(char state, String comp) {

    switch (state) {
        case '>':
            audioVolumeUp();
            sendAudioVolumeLevel();
            break;
        case '<':
            audioVolumeDown();
            sendAudioVolumeLevel();
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
                audioTogglePower();
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
// Amplifier Commands
// ******************************

void audioPowerOn() {
    digitalWrite(PIN_STBY, HIGH);
    IS_AUDIO_POWERED = true;
    audioVolumeSet(VOL_LEVEL_DEFAULT);
    mqttPublish("status/power", "on");
    led.on();
}

void audioPowerOff() {
    audioVolumeSet(0);
    IS_AUDIO_POWERED = false;
    digitalWrite(PIN_STBY, LOW);
    mqttPublish("status/power", "off");
    led.off();
}

void audioTogglePower() {
    if (IS_AUDIO_POWERED) {
        audioPowerOff();
    } else {
        audioPowerOn();
    }
}

void audioVolumeReset() {
    for (int i=0; i<=100; i++) {
        audioVolumeDown();
        delay(10);
    }
    VOL_LEVEL = 0;
    sendAudioVolumeLevel();
}

void audioVolumeUp() {
    delay(5);
    digitalWrite(PIN_VOLUP, LOW);
    delay(5);
    digitalWrite(PIN_VOLUP, HIGH);
    if (VOL_LEVEL < 64) {
        VOL_LEVEL++;
    }
}

void audioVolumeDown() {
    delay(5);
    digitalWrite(PIN_VOLDN, LOW);
    delay(5);
    digitalWrite(PIN_VOLDN, HIGH);
    if (VOL_LEVEL > 0) {
        VOL_LEVEL--;
    }
}


void sendAudioVolumeLevel() {
    char level [2];
    sprintf (level, "%d", VOL_LEVEL);
    mqttPublish("status/vol", level);
}

void audioVolumeSet(int level) {
    if (level > VOL_LEVEL) {
        for (int i=VOL_LEVEL; i<level; i++) {
            audioVolumeUp();
            delay(10);
        }
    } else {
        for (int i=VOL_LEVEL; i>level; i--) {
            audioVolumeDown();
            delay(10);
        }
    }
    sendAudioVolumeLevel();
}
