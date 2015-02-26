#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-ldr.h"
#include "../_inc/component-dac.h"
#include "../_inc/component-adc.h"

// ******************************
// Definitions
// ******************************

int ldrSteps = 32;
int bitDepth = 12;
float refVoltage = 3.2;

int calibrateInc = 0;
int calibrateStep = 0;
char* calibrateName;
float calibrateVoltageStep = refVoltage / ldrSteps;

SimpleTimer timer;


// ******************************
// Function Prototype Definitions
// ******************************

void compare1();
void compare2();
void calibrateStart();
void calibrateRun();

// ******************************
// Class instantiation
// ******************************

Dac dac1("dac1", 0x63, bitDepth, refVoltage);
Adc adc1("adc1", A1, bitDepth, refVoltage);
Ldr ldr1("ldr1", dac1, adc1, 32);

Dac dac2("dac2", 0x62, bitDepth, refVoltage);
Adc adc2("adc2", A2, bitDepth, refVoltage);
Ldr ldr2("ldr2", dac2, adc2, 32);

void setup() {

    coreSetup();
    mqttSetup("tester");

    //dac1.onChange(compare1);
    //dac2.onChange(compare2);

}

// ******************************
// Main Loop
// ******************************

void loop() {
    mqttLoop();
    timer.run();
}

Ldr *whichLdr;
float goalV;
float currV;
float prevV;
bool run;
int wait;
int min = 1750;
int max = 2250;

void calibrateRun() {

    currV = whichLdr->adc.getVoltage();

    if (currV >= goalV) {

        mqttStatus(calibrateName, "step", calibrateStep);
        mqttStatus(calibrateName, "goalV", goalV);
        mqttStatus(calibrateName, "currV", currV);
        whichLdr->adc.compare(calibrateInc);

        if (calibrateStep <= ldrSteps + 1) {
            calibrateStep++;
            goalV = calibrateStep * calibrateVoltageStep;
            wait = 500;
        } else {
            run = false;
        }

    } else {
        wait = 25;
    }

    if (run && calibrateInc < max) {
        calibrateInc++;
        whichLdr->dac.setValue(calibrateInc);
        timer.setTimeout(wait, calibrateRun);
    } else {
        mqttAction(calibrateName, "calibrate-stop");
    }

    prevV = currV;

}

void calibrate(int which) {

    if (which == 1) {
        whichLdr = &ldr1;
    } else if (which == 2) {
        whichLdr = &ldr2;
    }

    run = true;
    calibrateStep = 0;
    calibrateInc = min;
    whichLdr->dac.setValue(0);
    calibrateName = whichLdr->name;
    goalV = calibrateStep * calibrateVoltageStep;
    mqttAction(calibrateName, "calibrate-start");
    timer.setTimeout(1000, calibrateRun);

}

void compare1() {
    adc1.read();
    adc1.compare(dac1.getValue());
}

void mqttCustomMessageHandler(MqttMessage msg) {

    ldr1.mqtt(msg);
    ldr2.mqtt(msg);

    if (msg.isFor("ldr1")) {
        if (msg.is("calibrate")) {
            calibrate(1);
        }
    }

    if (msg.isFor("ldr2")) {
        if (msg.is("calibrate")) {
            calibrate(2);
        }
    }


}