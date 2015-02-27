#include "core-constants.h"
#include "core-utils.h"
#include "lib-memory.h"
#include "lib-timer.h"
#include "lib-mqtt.h"
#include "lib-mymqtt.h"

void coreSetup() {

    RGB.control(true);
    RGB.brightness(64);
    RGB.control(false);
    Serial.begin(9600);

}



void defaultKnobCheck(char state, char* comp) {

    if (IS_CONNECTED) {

        switch (state) {
            case '>':
                mqttAction(comp, "up");
                break;

            case '<':
                mqttAction(comp, "down");
                break;
        }
    }
}

void defaultButtonCheck(char state, char* comp) {

    if (IS_CONNECTED) {

        switch (state) {
            case 'P':
                mqttAction(comp, "press");
                break;
            case 'H':
                mqttAction(comp, "hold");
                break;
       }
    }
}