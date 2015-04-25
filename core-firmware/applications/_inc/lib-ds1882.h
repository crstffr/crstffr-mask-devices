#ifndef DS1882_h
#define DS1882_h

class DS1882
{
    public:
        DS1882(char* name);
        void mqtt(MqttMessage msg);
        void setLevel(int level);
        void sendStatus();
        int  level();
        void setup();
        void mute();
        void down();
        void up();

    private:
        char* _name;
        int _level;
        byte _address;
        bool _cfgUseEEPROM;
        bool _cfgUseZeroCross;
        bool _cfgUse32Positions;
        int _r0;
        int _r1;
        int _r2;

};

#endif

DS1882::DS1882(char* name)
{
    _name = name;
    _r0 = 0;
    _r1 = 0;
    _r2 = 0;
}

void DS1882::setup() {

    _level = 0;
    _address = 40;

    pinMode(D2, OUTPUT);
    digitalWrite(D2, HIGH);

    Wire.setSpeed(CLOCK_SPEED_400KHZ);
    Wire.stretchClock(true);
    Wire.begin();

    Wire.beginTransmission(_address);
    Wire.write(0b10000110);
    Wire.write(1);
    _r0 = Wire.endTransmission(true);

}

void DS1882::setLevel(int level) {

    if (level > 63) { level = 63; }
    if (level < 0)  { level =  0; }
    _level = level;

    // Volume settings are opposite our usual thinking.
    // 0 = full volume, 63 = mute.  So flip this around.

    int setting = 63 - level;

    // Send a command to the DS1882 for the first pot

    Wire.beginTransmission(_address);
    Wire.write(setting);
    _r1 = Wire.endTransmission(true);

    delay(25);

    // Send a command to the DS1882 for the second pot

    byte v1 = 0b01000000 | setting;
    Wire.beginTransmission(_address);
    Wire.write(v1);
    _r2 = Wire.endTransmission(true);

    sendStatus();

}

void DS1882::mute() {
    setLevel(0);
}

void DS1882::up() {
    setLevel(++_level);
}

void DS1882::down() {
    setLevel(--_level);
}

void DS1882::sendStatus() {
    mqttStatus("volume", "level", _level);
}

int DS1882::level() {
    return _level;
}

void DS1882::mqtt(MqttMessage msg) {

    if (!msg.isFor(_name)) { return; }

    mqttLog("DS1882/forme", msg.topic());

    if (msg.isStatusRequest()) {
        sendStatus();
        return;
    }

    if (msg.isCommand("up")) {
        up();
    }

    if (msg.isCommand("down")) {
        down();
    }

    if (msg.isCommand("mute")) {
        mute();
    }

    if (msg.isCommand("level")) {
        setLevel(msg.intVal());
        return;
    }

}