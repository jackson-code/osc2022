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


int str_token_count(const char *s, char delimiter)
{
	// calculate # of delimiter
	// int num = 1;
	// while (*s) {
	// 	if (*s == delimiter)
	// 		num++;
	// 	s++;
	// }
	// return num;

	while (*s == delimiter)
	{
		s++;
	}

	int num = 0;
	while (*s) {
		if (*s == delimiter)
			num++;
		s++;
	}

	while (*(s-1) == delimiter)
	{
		num--;
	}
	
	return ++num;
}

// devide tokens by space ' '
void str_token(const char *s, char **tokens, char delimiter)
{
	int token_idx = 0;
	int num = str_token_count(s, delimiter);

	while (*s == delimiter)
	{
		s++;
	}

	while (token_idx < num) {
		char *token = tokens[token_idx];
		while (*s != delimiter && *s != '\0')
			*token++ = *s++;
		*token = '\0';
		s++;			// skip delimiter
		token_idx++;	// next token
	}
}












