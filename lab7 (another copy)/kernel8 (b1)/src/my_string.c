#include "my_string.h"

int str_cpy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return i;
}

int str_cmp(const char *s, const char *t)
{
	for ( ; *s == *t; s++, t++) {
		if (*s == '\0')
			return 0;
	}
	return *s - *t;
}

void str_clear(char *s)
{
	while (*s != '\0') {
		*s++ = '\0';
	}
}

int str_len(const char* s)
{
    int len = 0;
    while(s[len] != '\0'){
        len++;
    }
    return len;
}


int str_token_count(char *s)
{
	// calculate # of ' '
	int num = 1;
	while (*s) {
		if (*s == ' ')
			num++;
		s++;
	}
	return num;
}

// devide tokens by space ' '
void str_token(char *s, int idx, char *token)
{
	int space_num = 0;
	while (*s) {
		if (space_num == idx) {
			while (*s != ' ' && *s != '\0')
				*token++ = *s++;
			*token = '\0';
			break;
		}
		else {
			while (*s != ' ')
				s++;
			s++;				// skip space
			space_num++;		// next token
		}
	}
}












