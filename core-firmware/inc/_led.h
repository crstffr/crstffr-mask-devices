#ifndef LED_h
#define LED_h

class LED
{
    public:
        LED(int pinR, int pinG, int pinB);
        void on();
        void off();
        void loop();
        void rgb(int r, int g, int b);
        void intensity(int value);
        void setMaxIntensity(int value);
        void color(char* name);
        void dim();
        void fade();
        void blank();
        void blink();
        void change();
        void calcFade();
        void calcBlink();
        int getState();
        int getIntensity();
        char* getColor();

    private:
        int _pinR;
        int _pinG;
        int _pinB;
        int _r;
        int _g;
        int _b;
        int _state;
        int _blink;
        char* _color;
        bool _fading;
        bool _blinking;
        int _intensity;
        int _intensityMax;
        unsigned long _now;
        unsigned long _fadeTimer;
        unsigned long _blinkTimer;
        unsigned long _blinkGap;
};

#endif