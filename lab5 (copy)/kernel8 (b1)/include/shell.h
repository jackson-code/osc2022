#ifndef _SHELL_H
#define _SHELL_H

#include "uart.h"
#include "mailbox.h"
#include "my_string.h"
#include "command.h"
#include "cpio.h"
#include "allocator.h"
#include "dtb_parser.h"
#include "el1.h"
#include "timer.h"
#include "irq.h"
#include "mm.h"
#include "thread.h"



void shell_get_command(char *cmd, int lim);
void shell_async_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

#endif
