int str_cmp(char *s, char *t)
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

int str_len(const char* s){
    int len = 0;
    while(s[len] != '\0'){
        len++;
    }
    return len;
}
