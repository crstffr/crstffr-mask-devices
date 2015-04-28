#ifndef ADG409_h
#define ADG409_h

class ADG409
{
    public:
        ADG409(int pin1, int pin2);
        void select(int input);
        int getSelection();

    private:
        int _pin1;
        int _pin2;
        int _input;
};

ADG409::ADG409(int pin1, int pin2) {

    _pin1 = pin1;
    _pin2 = pin2;

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);

}

int ADG409::getSelection() {
    return _input;
}

void ADG409::select(int input) {

    _input = (input >= 1 && input <= 4) ? input : 1;

    switch (_input) {
        case 1:
            digitalWrite(_pin1, LOW);
            digitalWrite(_pin2, LOW);
            break;

        case 2:
            digitalWrite(_pin1, HIGH);
            digitalWrite(_pin2, LOW);
            break;

        case 3:
            digitalWrite(_pin1, LOW);
            digitalWrite(_pin2, HIGH);
            break;

        case 4:
            digitalWrite(_pin1, HIGH);
            digitalWrite(_pin2, HIGH);
            break;
    }
}

#endif
