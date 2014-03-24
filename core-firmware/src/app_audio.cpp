#include "application.h"
#include "_quadencoder.h"
#include "_Si4703.h"
#include "_button.h"
#include "_mytcp.h"
#include "_led.h"

// ******************************
// Definitions
// ******************************

int PIN_SCL   = D0;
int PIN_SDA   = D1;
int PIN_RST   = D2;
int PIN_STBY  = D3;
int PIN_MUTE  = D4;
int PIN_POWER = D5;
int PIN_VOLUP = D6;
int PIN_VOLDN = D7;

int RGB_LED[3] = {A0, A1, A2};
int PIN_LEDBTN = A3;
int PIN_ENC1   = A4;
int PIN_ENC2   = A5;
int PIN_ENCBTN = A6;
int PIN_MSCBTN = A7;

String STATE_ON  = "1";
String STATE_OFF = "0";

String ACTIVITY_DEBUG = "0";
String ACTIVITY_PRESS = "1";
String ACTIVITY_HOLD  = "2";
String ACTIVITY_ON    = "3";
String ACTIVITY_OFF   = "4";
String ACTIVITY_UP    = "5";
String ACTIVITY_DOWN  = "6";
String ACTIVITY_TURNCW  = "7";
String ACTIVITY_TURNCCW = "8";

String COMPONENT_LEDBTN = "1";
String COMPONENT_ENCBTN = "5";
String COMPONENT_MSCBTN = "6";
String COMPONENT_KNOB   = "7";
String COMPONENT_MUTE   = "2";
String COMPONENT_POWER  = "3";
String COMPONENT_VOLUME = "4";

char COMMAND_STATUS    = 'S';
char COMMAND_LEDOFF    = 'O';
char COMMAND_LEDWHITE  = 'W';
char COMMAND_LEDRED    = 'R';
char COMMAND_LEDGREEN  = 'G';
char COMMAND_LEDBLUE   = 'B';
char COMMAND_LEDCYAN   = 'C';
char COMMAND_LEDMAG    = 'N';
char COMMAND_LEDYEL    = 'Y';
char COMMAND_LEDBLINK  = 'L';
char COMMAND_LEDFADE   = 'F';
char COMMAND_MUTETOG   = 'M';
char COMMAND_POWERTOG  = 'P';
char COMMAND_POWEROFF  = '0';
char COMMAND_POWERON   = '1';
char COMMAND_MUTEOFF   = '2';
char COMMAND_MUTEON    = '3';
char COMMAND_VOLUMEUP  = 'U';
char COMMAND_VOLUMEDN  = 'D';
char COMMAND_VOLUMELOW = 'E';

int  VOL_LEVEL = 0;
bool IS_CONNECTED_TO_SERVER = false;
bool IS_AUDIO_POWERED = false;
bool IS_AUDIO_MUTED = false;

// ******************************
// Function Prototype Definitions
// ******************************

int connect(String ip);
int disconnect(String params);
void audioMuteOn();
void audioMuteOff();
void audioToggleMute();
void audioPowerOn();
void audioPowerOff();
void audioTogglePower();
void audioVolumeUp();
void audioVolumeDown();
void audioVolumeReset();
void sendAudioVolumeLevel();
void audioVolumeSet(int level);
void audioVolumeLow();
void status();

void checkKnob(char state, String component);
void checkButton(char state, String component);

// ******************************
// Class instantiation
// ******************************

MyTCP mytcp;
QuadEncoder knob(PIN_ENC1, PIN_ENC2);
LED led(RGB_LED[0], RGB_LED[1], RGB_LED[2]);
Button ledbtn(PIN_LEDBTN, INPUT_PULLUP);
Button encbtn(PIN_ENCBTN, INPUT_PULLUP);
Button mscbtn(PIN_MSCBTN, INPUT_PULLUP);
Si4703 radio(PIN_RST, PIN_SDA, PIN_SCL);

void setup()
{

    Spark.function("connect", connect);
    Spark.function("disconnect", disconnect);

    //pinMode(RGB_LED[0], INPUT);
    //pinMode(RGB_LED[1], INPUT);
    //pinMode(RGB_LED[2], INPUT);

    pinMode(PIN_POWER, OUTPUT);
    pinMode(PIN_VOLUP, OUTPUT);
    pinMode(PIN_VOLDN, OUTPUT);
    pinMode(PIN_MUTE,  OUTPUT);
    pinMode(PIN_STBY,  OUTPUT);
    pinMode(PIN_RST,   OUTPUT);
    /*
    pinMode(PIN_LEDBTN, INPUT_PULLUP);
    pinMode(PIN_ENCBTN, INPUT_PULLUP);
    pinMode(PIN_ENC1,   INPUT_PULLUP);
    pinMode(PIN_ENC2,   INPUT_PULLUP);
    */

    digitalWrite(PIN_VOLUP, LOW);
    digitalWrite(PIN_VOLDN, LOW);
    digitalWrite(PIN_POWER, HIGH);

    radio.powerOn();
    radio.setVolume(12);
    radio.setChannel(879);

    delay(500);

    digitalWrite(PIN_VOLUP, HIGH);
    digitalWrite(PIN_VOLDN, HIGH);

    audioMuteOff();
    audioPowerOn();

}

// ******************************
// Main Loop
// ******************************

void loop()
{

    mytcp.tick();

    checkKnob(knob.state(), COMPONENT_KNOB);
    checkButton(ledbtn.state(), COMPONENT_LEDBTN);
    checkButton(encbtn.state(), COMPONENT_ENCBTN);
    checkButton(mscbtn.state(), COMPONENT_MSCBTN);

    char read = mytcp.read();

    IS_CONNECTED_TO_SERVER = mytcp.status();

    if (IS_CONNECTED_TO_SERVER) {
        led.color("green");
    } else {
        led.color("red");
    }

    if (read == COMMAND_LEDOFF) {
        //led.off();
    } else if (read == COMMAND_LEDWHITE) {
        //led.color("white");
    } else if (read == COMMAND_LEDRED) {
        //led.color("red");
    } else if (read == COMMAND_LEDGREEN) {
        //led.color("green");
    } else if (read == COMMAND_LEDBLUE) {
        //led.color("blue");
    } else if (read == COMMAND_LEDCYAN) {
        //led.color("cyan");
    } else if (read == COMMAND_LEDMAG) {
        //led.color("magenta");
    } else if (read == COMMAND_LEDYEL) {
        //led.color("yellow");
    } else if (read == COMMAND_LEDBLINK) {
        //led.blink();
    } else if (read == COMMAND_LEDFADE) {
        //led.fade();
    } else if (read == COMMAND_MUTEON) {
        audioMuteOn();
    } else if (read == COMMAND_MUTEOFF) {
        audioMuteOff();
    } else if (read == COMMAND_MUTETOG) {
        audioToggleMute();
    } else if (read == COMMAND_POWERON) {
        audioPowerOn();
    } else if (read == COMMAND_POWEROFF) {
        audioPowerOff();
    } else if (read == COMMAND_POWERTOG) {
        audioTogglePower();
    } else if (read == COMMAND_VOLUMEUP) {
        audioVolumeUp();
        sendAudioVolumeLevel();
    } else if (read == COMMAND_VOLUMEDN) {
        audioVolumeDown();
        sendAudioVolumeLevel();
    } else if (read == COMMAND_VOLUMELOW) {
        audioVolumeLow();
    } else if (read == COMMAND_STATUS) {
        status();
    }

}

// ******************************
// Status
// ******************************

void status() {

    if (IS_AUDIO_POWERED) {
        mytcp.sendValue(COMPONENT_POWER, STATE_ON);
    } else {
        mytcp.sendValue(COMPONENT_POWER, STATE_OFF);
    }

    if (IS_AUDIO_MUTED) {
        mytcp.sendValue(COMPONENT_MUTE, STATE_ON);
    } else {
        mytcp.sendValue(COMPONENT_MUTE, STATE_OFF);
    }

    sendAudioVolumeLevel();

}

// ******************************
// Spark Cloud Functions
// ******************************

int connect(String ip) {
    mytcp.setIP(ip);
    return mytcp.connect();
}

int disconnect(String params) {
    return mytcp.disconnect();
}

// ******************************
// Knob Handling
// ******************************

void checkKnob(char state, String component) {

    if (IS_CONNECTED_TO_SERVER) {

        switch (state) {
            case '>':
                mytcp.sendAction(component, ACTIVITY_TURNCW);
                break;
            case '<':
                mytcp.sendAction(component, ACTIVITY_TURNCCW);
                break;
        }


    } else {

        switch (state) {
            case '>':
                audioVolumeUp();
                break;
            case '<':
                audioVolumeDown();
                break;
        }

    }

}

// ******************************
// Button Handling
// ******************************

void checkButton(char state, String component) {
    if (IS_CONNECTED_TO_SERVER) {
        switch (state) {
            case 'P':
                mytcp.sendAction(component, ACTIVITY_PRESS);
                break;
            case 'H':
                mytcp.sendAction(component, ACTIVITY_HOLD);
                break;
        }
    } else {
        if (component == COMPONENT_ENCBTN) {
           switch (state) {
                case 'P':
                    audioTogglePower();
                    break;
            }
        }
        if (component == COMPONENT_LEDBTN) {
           switch (state) {
                case 'P':
                    led.dim();
                    break;
            }
        }
        if (component == COMPONENT_MSCBTN) {
           switch (state) {
                case 'P':

                    break;
            }
        }
    }
}

// ******************************
// Amplifier Commands
// ******************************

void audioMuteOn() {
    IS_AUDIO_MUTED = true;
    digitalWrite(PIN_MUTE, LOW);
    mytcp.sendAction(COMPONENT_MUTE, ACTIVITY_ON);
}

void audioMuteOff() {
    IS_AUDIO_MUTED = false;
    digitalWrite(PIN_MUTE, HIGH);
    mytcp.sendAction(COMPONENT_MUTE, ACTIVITY_OFF);
}

void audioToggleMute() {
    if (IS_AUDIO_MUTED) {
        audioMuteOff();
    } else {
        audioMuteOn();
    }
}

void audioPowerOn() {
    digitalWrite(PIN_STBY, HIGH);
    IS_AUDIO_POWERED = true;
    audioVolumeSet(20);
    led.color("green");
    mytcp.sendAction(COMPONENT_POWER, ACTIVITY_ON);
}

void audioPowerOff() {
    audioVolumeSet(0);
    IS_AUDIO_POWERED = false;
    digitalWrite(PIN_STBY, LOW);
    led.off();
    mytcp.sendAction(COMPONENT_POWER, ACTIVITY_OFF);
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

void audioVolumeLow() {
    audioVolumeSet(20);
    audioMuteOff();
}

void sendAudioVolumeLevel() {
    String vol = "";
    vol += VOL_LEVEL;
    mytcp.sendValue(COMPONENT_VOLUME, vol);
}

void audioVolumeSet(int level) {
    if (level > VOL_LEVEL) {
        for (int i=VOL_LEVEL; i<=level; i++) {
            audioVolumeUp();
            delay(10);
        }
    } else {
        for (int i=VOL_LEVEL; i>=level; i--) {
            audioVolumeDown();
            delay(10);
        }
    }
    sendAudioVolumeLevel();
}
