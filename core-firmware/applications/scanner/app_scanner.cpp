#include "application.h"
#include "../_inc/core-common.h"
#include "../_inc/component-button.h"

byte error;
byte address;
int timerNum;
void startScan();
void scan();

SimpleTimer timer;
Button btn("btn", A2, INPUT_PULLUP);

void setup()
{
  coreSetup();
  mqttSetup(DEVICE_TYPE_SMARTAMP);
  btn.onPress(startScan);

  Wire.setSpeed(CLOCK_SPEED_400KHZ);
  Wire.stretchClock(true);
  Wire.begin();

  timerNum = timer.setInterval(10, scan);
  timer.disable(timerNum);

}

void loop() {
    mqttLoop();
    btn.loop();
    timer.run();
}

void startScan() {

  address = 0x01;
  timer.enable(timerNum);
  mqttStatus("i2c", "msg", "Scanning...");

}

void scan() {

    address++;

    if (address > 0x7f) {

        mqttStatus("i2c", "msg", "Scanning Complete");
        timer.disable(timerNum);

    } else {

        Wire.beginTransmission(address);
        Wire.write(0x00);
        error = Wire.endTransmission(true);

        if (error == 0) {
          mqttStatus("i2c", "msg", "Device found");
          mqttStatus("i2c", "msg", address);
        } else {
          //mqttStatus("i2c", "msg", error);
        }
    }
}


void mqttCustomMessageHandler(MqttMessage msg) {}