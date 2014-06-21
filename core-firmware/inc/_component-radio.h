#include "application.h"

#ifndef Si4703
#include "_lib-Si4703.h"
#endif

#ifndef Radio_h
#define Radio_h

class Radio
{
    public:
        Radio(int rst, int sda, int scl);
        void begin();
        void powerOn();
        void setVolume(int level);
        void setStation(int station);
        void sendStatus();
    private:
        int _volume;
        int _station;
        Si4703 _radio;
};

Radio::Radio(int rst, int sda, int scl) : _radio(rst, sda, scl) {

}

void Radio::powerOn() {
    _radio.powerOn();
}

void Radio::setVolume(int level) {
    if (_volume != level) {
        _volume = level;
        _radio.setVolume(level);
    }
}

void Radio::setStation(int station) {
    if (_station != station) {
        _station = station;
        _radio.setChannel(station);
    }
}

void Radio::sendStatus() {

    if (IS_CONNECTED) {

        char vol[2] = "";
        char chn[3] = "";

        itoa(_volume, vol, 10);
        itoa(_station, chn, 10);

        mqttPublish("status/radio/volume", vol);
        mqttPublish("status/radio/station", chn);

    }

}

#endif
