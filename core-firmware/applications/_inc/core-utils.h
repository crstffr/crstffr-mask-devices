
void noop() {}          // no op fn with no attr
void noopI(int i) {}    // no op fn with int attr
void noopC(char* c) {}  // no op fn with char attr
void noopF(float f) {}  // no op fn with float attr

typedef void (*callback)(void);     // callback with no attr
typedef void (*callbackI)(int i);   // callback with int attr
typedef void (*callbackC)(char* c); // callback with char attr
typedef void (*callbackF)(float f); // callback with float attr

extern char* itoa(int i, char* buffer, unsigned char radix);

bool equals(char* one, char* two) {
    return (strcmp(one, two) == 0);
}

