#include "application.h"

#ifndef Si4703_h
#include "lib-Si4703.h"
#endif

#ifndef Radio_h
#define Radio_h

class Radio
{
    public:
        Radio(char* name, int rst, int sda, int scl);
        void begin();
        void powerOn();
        void setVolume(int level);
        void setStation(int station);
        void sendStatus();
        void skipStation();
    private:
        char* _name;
        int _volume;
        int _station;
        Si4703 _radio;
};

Radio::Radio(char* name, int rst, int sda, int scl) : _radio(rst, sda, scl) {
    _name = name;
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

void Radio::skipStation() {
    switch (_station) {
        case 879:
            this->setStation(893);
            break;
        case 893:
            this->setStation(911);
            break;
        case 911:
            this->setStation(937);
            break;
        case 937:
            this->setStation(971);
            break;
        case 971:
            this->setStation(1013);
            break;
        default:
            this->setStation(879);
            break;
    }
}

void Radio::sendStatus() {

    if (IS_CONNECTED) {

        char vol[2] = "";
        char chn[3] = "";

        itoa(_volume, vol, 10);
        itoa(_station, chn, 10);

        mqttStatus("radio/volume", vol);
        mqttStatus("radio/station", chn);

    }

}

#endif
