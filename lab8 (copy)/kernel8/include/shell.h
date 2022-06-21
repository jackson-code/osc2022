#ifndef _SHELL_H_
#define _SHELL_H_

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

void shell_get_command(char *cmd, int lim);
void shell_async_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

#endif
