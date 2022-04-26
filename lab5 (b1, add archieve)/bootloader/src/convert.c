#include "my_math.h"

char *itoa(int value, char *s) {
    int idx = 0;
    if (value < 0) {
        value *= -1;
        s[idx++] = '-';
    }

    char tmp[10];
    int tidx = 0;
    do {
        tmp[tidx++] = '0' + value % 10;
        value /= 10;
    } while (value != 0 && tidx < 11);

    // reverse tmp
    int i;
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';

    return s;
}

char *ftoa(float value, char *s) {
    int idx = 0;
    if (value < 0) {
        value = -value;
        s[idx++] = '-';
    }

    int ipart = (int)value;
    float fpart = value - (float)ipart;

    // convert ipart
    char istr[11];  // 10 digit
    itoa(ipart, istr);

    // convert fpart
    char fstr[7];  // 6 digit
    fpart *= (int)pow(10, 6);
    itoa((int)fpart, fstr);

    // copy int part
    char *ptr = istr;
    while (*ptr) s[idx++] = *ptr++;
    s[idx++] = '.';
    // copy float part
    ptr = fstr;
    while (*ptr) s[idx++] = *ptr++;
    s[idx] = '\0';

    return s;
}


// Convert ascii code which represent hex to integer
// size = # bytes
unsigned long ahtoi(char* addr, const int size){
    unsigned long res = 0;
    char c;
    for(int i = 0 ;i < size; ++i){
    	// little edian
        res <<= 4;		// a hex occupy 4 bits
        c = *addr++;	// get an ascii code in hex
        
        if(c >= '0' && c <= '9') res += c - '0';
        else if(c >= 'A' && c <= 'F') res += c - 'A' + 10;
        else if(c >= 'a' && c <= 'f') res += c - 'a' + 10;
    }
    return res;
}

// convert big endian to little endian
// Unsigned 32-bit conversion 
unsigned int btol(unsigned int num){
	return ((num>>24)&0xff) | 		// move byte 3 to byte 0
        	((num<<8)&0xff0000) | 	// move byte 1 to byte 2
            ((num>>8)&0xff00) | 	// move byte 2 to byte 1
            ((num<<24)&0xff000000); // byte 0 to byte 3
}
