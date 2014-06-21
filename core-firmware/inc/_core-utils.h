
void noop() {}

typedef void (*callback)(void);

extern char* itoa(int i, char* buffer, unsigned char radix);

bool equals(char* one, char* two) {
    return (strcmp(one, two) == 0);
}

