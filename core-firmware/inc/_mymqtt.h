#ifndef MyMQTT_h
#define MyMQTT_h

class MyMQTT
{
    public:
        MyMQTT(PubSubClient mqtt, void(*)(String, String));

    private:
        void (*callback)(String, String);
        void byteToChar(byte array[], char buffer[], byte len);
};

#endif