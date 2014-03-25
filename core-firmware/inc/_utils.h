
void byteToChar(byte array[], char buffer[], byte len) {
    for (byte i = 0; i < len; i++) {
        char c = array[i];
        char temp[ 2 ];
        temp[ 0 ] = c;
        temp[ 1 ] = '\0';
        strcat (buffer, temp);
    }
}

