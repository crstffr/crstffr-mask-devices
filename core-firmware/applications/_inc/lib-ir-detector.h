#ifndef IR_H
#define IR_H

#ifndef NULL
#define NULL 0
#endif

// --------------------------- IR Detector lib -------------------------- //
// IR Detector data
class IRDetector {

    private:
      RingBuffer buffer;
      unsigned int command;
      unsigned long curTime;
      unsigned long elapsed;
      unsigned int numBitsAvailable;
      unsigned long lastBitTimestamp;

    public:
      IRDetector(int pin);
      void update();
      unsigned int pop() { return buffer.pop(); }
      int available() { return buffer.available(); }

};

IRDetector::IRDetector(int pin) {
    pinMode(pin, INPUT);
    buffer.init();
    command = 0;
    lastBitTimestamp = 0;
    numBitsAvailable = 0;
}

void IRDetector::update() {

  curTime = micros();
  elapsed = curTime - lastBitTimestamp;
  lastBitTimestamp = curTime;
  numBitsAvailable++;

  if (elapsed < 1500) { // Zero

    command <<= 1;

  } else if (elapsed < 2500) { // One

    command <<= 1;
    command++;

  } else { // Start new command

    // command = 0; // No need as we work in 16-bit chunks
    numBitsAvailable = 0;

  }

  // Command is finished on 16 bits.
  if (numBitsAvailable == 16) {
    buffer.push(command);
    numBitsAvailable = 0;
  }

}

#endif