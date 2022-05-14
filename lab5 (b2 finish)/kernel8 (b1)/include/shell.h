#ifndef _SHELL_H
#define _SHELL_H

void shell_get_command(char *cmd, int lim);
void shell_async_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

#endif
