#include "_core-constants.h"
#include "_core-utils.h"
#include "_lib-timer.h"
#include "_lib-mqtt.h"
#include "_lib-mymqtt.h"

void defaultKnobCheck(char state, int comp) {

    if (IS_CONNECTED) {

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

void defaultButtonCheck(char state, int comp) {

    if (IS_CONNECTED) {

        switch (state) {
            case 'P':
                mqttPublish("action/button","press");
                break;
            case 'H':
                mqttPublish("action/button","hold");
                break;
       }
    }
}