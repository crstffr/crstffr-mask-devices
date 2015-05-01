#ifndef QuadEncoder_h
#define QuadEncoder_h

class QuadEncoder
{
  public:
    QuadEncoder(char* name, int pin1, int pin2, int ppr);
    char state();
    void loop();
    void onUp(callback fn);
    void onDown(callback fn);
    void onChange(callbackI fn);
    void onComplete(callbackI fn);
    void checkTimer();
    void up();
    void dn();

  private:
    char* _name;
    bool _busy;
    int _pos;
    int _ppr;
    int _turn;
    int _pin1;
    int _pin2;
    int _val1;
    int _val2;
    int _oldPos;
    int _oldVal1;
    int _oldVal2;
    int _turnCount;
    int _accelMult;
    int _realChange;
    int _calcChange;
    unsigned int _startTime;

    callback _onUp;
    callback _onDown;
    callbackI _onChange;
    callbackI _onComplete;

};

QuadEncoder::QuadEncoder(char* name, int pin1, int pin2, int ppr)
{
    _name = name;
    _busy = false;

    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);

    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, HIGH);

    _ppr = ppr;
    _pin1 = pin1;
    _pin2 = pin2;

    _pos = 0;
    _turn = 0;
    _val1 = 1;
    _val2 = 1;
    _oldPos = 0;
    _oldVal1 = 1;
    _oldVal2 = 1;
    _turnCount = 0;
    _startTime = 0;
    _accelMult = 1;
    _realChange = 0;
    _calcChange = 0;

    _onUp = noop;
    _onDown = noop;
    _onChange = noopI;
    _onComplete = noopI;

}

void QuadEncoder::onUp(callback fn) {
    _onUp = fn;
}

void QuadEncoder::onDown(callback fn) {
    _onDown = fn;
}

void QuadEncoder::onChange(callbackI fn) {
    _onChange = fn;
}

void QuadEncoder::onComplete(callbackI fn) {
    _onComplete = fn;
}

void QuadEncoder::loop() {

  checkTimer();

  _val1 = digitalRead(_pin1);
  _val2 = digitalRead(_pin2);

  if ( _val1 != _oldVal1 || _val2 != _oldVal2) {

    _oldVal1=_val1;
    _oldVal2=_val2;

    if ( _val1 == 1 ) {
      if (_val2 == 1) {
        _pos = 0;
      } else if (_val2 == 0) {
        _pos = 3;
      }
    } else if (_val1 == 0) {
      if (_val2 == 1) {
        _pos = 1;
      } else if (_val2 == 0) {
        _pos = 2;
      }
    }

    _turn = _pos-_oldPos;
    _oldPos = _pos;

    if (abs(_turn) != 2) {
      if (_turn == 1 || _turn == -3) {
        _turnCount++;
      } else if (_turn == -1 || _turn == 3) {
        _turnCount--;
      }
    }

    if (_pos == 0) {

      if (_turnCount > 0) {
        up();
      } else if (_turnCount < 0) {
        dn();
      }

      _turnCount = 0;

    }
  }
}

void QuadEncoder::checkTimer() {

    if (_busy) {

        if (_startTime + 125 < millis()) {
            int change = abs(_realChange);
            if (change >= _ppr / 2) {
                _accelMult = 3;
            } else if (change >= _ppr / 4) {
                _accelMult = 2;
            } else {
                _accelMult = 1;
            }
        }

        // Enough time has gone by, lets clean up

        if (_startTime + 300 < millis()) {
            _onComplete(_calcChange);
            _calcChange = 0;
            _realChange = 0;
            _busy = false;
        }

    } else {

        _accelMult = 1;

    }

}


void QuadEncoder::up() {

    _busy = true;
    _realChange++;
    int change = 1 * _accelMult;
    _calcChange += change;
    _startTime = millis();

    //_onUp();
    _onChange(change);
}

void QuadEncoder::dn() {

    _busy = true;
    _realChange--;
    int change = -1 * _accelMult;
    _calcChange += change;
    _startTime = millis();

    //_onDown();
    _onChange(change);
}

#endif

