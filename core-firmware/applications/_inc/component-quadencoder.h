#ifndef QuadEncoder_h
#define QuadEncoder_h

class QuadEncoder
{
  public:
    QuadEncoder(char* name, int pin1, int pin2);
    char state();
    void loop();
    void onUp(callback fn);
    void onDown(callback fn);
    void up();
    void dn();
  private:
    bool _moved;
    char* _name;
    int _inputPin1;
    int _inputPin2;
    int _val1;
    int _val2;
    int _oldVal1;
    int _oldVal2;
    int _pos;
    int _oldPos;
    int _turn;
    int _turnCount;

    callback _onUp;
    callback _onDown;
};

QuadEncoder::QuadEncoder(char* name, int pin1, int pin2)
{
    _name = name;
    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, HIGH);
    _inputPin1=pin1;
    _inputPin2=pin2;
    _val1=1;
    _val2=1;
    _oldVal1=1;
    _oldVal2=1;
    _pos=0;
    _oldPos=0;
    _turn=0;
    _turnCount=0;

    _onUp = noop;
    _onDown = noop;

}

void QuadEncoder::onUp(callback fn) {
    _onUp = fn;
}

void QuadEncoder::onDown(callback fn) {
    _onDown = fn;
}

char QuadEncoder::state()
{
  _moved = false;
  _val1 = digitalRead(_inputPin1);
  _val2 = digitalRead(_inputPin2);
  if ( _val1 != _oldVal1 || _val2 != _oldVal2) {
    _oldVal1=_val1;
    _oldVal2=_val2;
    if ( _val1 == 1 ) {
      if (_val2 == 1)
        _pos = 0;
      else if (_val2 == 0)
        _pos = 3;
    } else if (_val1 == 0){
      if (_val2 == 1)
        _pos = 1;
      else if (_val2 == 0)
        _pos = 2;
    }
    _turn = _pos-_oldPos;
    _oldPos = _pos;
    if (abs(_turn) != 2) {
      if (_turn == 1 || _turn == -3)
        _turnCount++;
      else if (_turn == -1 || _turn == 3)
        _turnCount--;
    }
    if (_pos==0){
      if (_turnCount>0){
        _turnCount=0;
        _moved = true;
        return '>';
      } else if (_turnCount<0){
        _moved = true;
        _turnCount=0;
        return '<';
      } else {
        _moved = false;
        _turnCount=0;
        return '-';
      }
    } else {
        return '-';
    }
  } else {
      return '-';
  }
}

void QuadEncoder::loop() {
    switch (this->state()) {
        case '>':
            up();
            break;
        case '<':
            dn();
            break;
    }
}

void QuadEncoder::up() {
    this->_onUp();
    if (IS_CONNECTED) {
        mqttAction(_name, "up");
    }
}

void QuadEncoder::dn() {
    this->_onDown();
    if (IS_CONNECTED) {
        mqttAction(_name, "down");
    }
}

#endif

