
void noop() {}

typedef void (*callback)(void);

extern char* itoa(int i, char* buffer, unsigned char radix);

bool equals(char* one, char* two) {
    return (strcmp(one, two) == 0);
}

// Convert a String to a char*
// I'm pretty sure this doesn't work but not sure why.

char* stoc(String s) {
    char c[s.length() + 1];
    s.toCharArray(c, s.length() + 1);
    return c;
}

