#ifndef _MY_STRING_H_
#define _MY_STRING_H_

int 	str_cmp(char *s, char *t);
void 	str_clear(char *s);
int 	str_len(const char* s);
int 	str_token_count(char *s);
void 	str_token(char *s, int idx, char *token);

#endif