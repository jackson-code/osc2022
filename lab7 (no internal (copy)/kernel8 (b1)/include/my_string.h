#ifndef _MY_STRING_H_
#define _MY_STRING_H_

int     str_cpy(char *dest, const char *src);
int 	str_cmp(const char *s, const char *t);
void 	str_clear(char *s);
int 	str_len(const char* s);
int 	str_token_count(const char *s, char delimiter);
void    str_token(const char *s, char **tokens, char delimiter);


#endif