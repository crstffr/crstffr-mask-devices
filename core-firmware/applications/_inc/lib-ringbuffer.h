#ifndef RingBuffer_h
#define RingBuffer_h

// Ring buffer for storing unsigned ints
class RingBuffer {
    private:
      int head;
      int tail;
      static const int BUF_SIZE_BITS = 7;
      static const int BUF_SIZE = (1 << BUF_SIZE_BITS);
      static const int BUF_SIZE_MASK = (BUF_SIZE - 1);
      unsigned int buffer[BUF_SIZE];

    public:
      void init() {
        head = tail = 0;
      }
      void push(unsigned int val) {
        buffer[head++] = val;
        head &= BUF_SIZE_MASK;
        if (head == tail) { // Overwriting tail
          tail++;
          tail &= BUF_SIZE_MASK;
        }
      }
      unsigned int pop() {  // Don't do this unless available == true
        unsigned int result = buffer[tail++];
        tail &= BUF_SIZE_MASK;
        return result;
      }
      int available() {
        return (head != tail);
      }
};

#endif