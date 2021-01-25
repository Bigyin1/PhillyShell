#ifndef PHILLYSHELL_TTY_H
#define PHILLYSHELL_TTY_H

#include "structs/str.h"
#include <limits.h>
#include <termios.h>

void tty_save ();
void tty_restore ();
void tty_setraw (char *term_name);
String *tty_readline ();

#endif // PHILLYSHELL_TTY_H
