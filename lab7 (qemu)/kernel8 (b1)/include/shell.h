#ifndef _SHELL_H_
#define _SHELL_H_

void shell_get_command(char *cmd, int lim);
void shell_async_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

#endif
