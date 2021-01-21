#ifndef PHILLYSHELL_TTY_H
#define PHILLYSHELL_TTY_H

#include "../errors/errors.h"
#include <limits.h>
#include <termios.h>

extern struct termios tty_default;
extern struct termios tty_fsh;

typedef struct
{
  char buf[MAX_INPUT];
} t_tty;

sh_ecode tty_init();
char *tty_readline (t_tty*);

#endif // PHILLYSHELL_TTY_H
