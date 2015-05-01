#include "core-constants.h"
#include "core-utils.h"
#include "lib-ringbuffer.h"
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