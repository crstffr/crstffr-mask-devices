#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-ldr.h"
#include "../_inc/component-dac.h"
#include "../_inc/component-adc.h"
#include "../_inc/component-lightspeed.h"

// ******************************
// Definitions
// ******************************

int ldrSteps = 32;
int bitDepth = 12;
float refVoltage = 3.3;

int calibrateInc = 0;
int calibrateStep = 1;
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
Ldr ldr1("L-Series", dac1, adc1, ldrSteps);

Dac dac2("dac2", 0x62, bitDepth, refVoltage);
Adc adc2("adc2", A2, bitDepth, refVoltage);
Ldr ldr2("L-Shunt", dac2, adc2, ldrSteps);

Lightspeed vol("vol", ldr1, ldr2, ldrSteps);

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
float prevV;
float currV;
float currDiff;
float prevDiff;

bool run;
int wait;
int min = 1800;
int max = 2500;

void calibrateRun() {

    currV = whichLdr->adc.getVoltage();

    if (prevV <= goalV && currV >= goalV) {

        prevDiff = goalV - prevV;
        currDiff = currV - goalV;

        mqttStatus(calibrateName, "step", calibrateStep);

        if (prevDiff > currDiff) {
            mqttStatus(calibrateName, "voltage", prevV);
            whichLdr->calibrate(calibrateStep, calibrateInc - 1);
            whichLdr->compare(calibrateInc - 1);
        } else {
            mqttStatus(calibrateName, "voltage", currV);
            whichLdr->calibrate(calibrateStep, calibrateInc);
            whichLdr->compare(calibrateInc);
        }

        if (calibrateStep < ldrSteps - 1) {
            wait = 150;
            calibrateStep++;
            goalV = calibrateStep * calibrateVoltageStep;
        } else {
            run = false;
        }

    } else {
        wait = 10;
    }

    if (run && calibrateInc < max) {
        calibrateInc++;
        whichLdr->dac.setValue(calibrateInc);
        timer.setTimeout(wait, calibrateRun);
    } else {
        whichLdr->calibrated();
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
    calibrateStep = 1;
    calibrateInc = min;
    whichLdr->reset();
    calibrateName = whichLdr->name;
    goalV = calibrateStep * calibrateVoltageStep;
    mqttAction(calibrateName, "calibrate-start");
    timer.setTimeout(2000, calibrateRun);

}

void compare1() {
    adc1.read();
    adc1.compare(dac1.getValue());
}

void mqttCustomMessageHandler(MqttMessage msg) {

    vol.mqtt(msg);

    if (msg.is("reset")) {
        ldr1.reset();
        ldr2.reset();
    }

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