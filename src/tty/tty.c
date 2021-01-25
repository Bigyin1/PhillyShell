#include "tty.h"
#include <termcap.h>
#include <unistd.h>
#include "../errors/errors.h"

static struct termios saved_term;

void
tty_save ()
{
  if (!isatty (STDIN_FILENO))
    errors_fatal ("tty error\n");
  if (tcgetattr (0, &saved_term) == -1)
    errors_fatal ("tty error\n");
}

void
tty_restore ()
{
  if (tcsetattr (0, TCSADRAIN, &saved_term) == -1)
    errors_fatal ("tty error\n");
}

#ifdef unix
static char term_buffer[2048];
#else
#define term_buffer 0
#endif

void
tty_setraw (char *term_name)
{
  if (!isatty (STDIN_FILENO))
    errors_fatal ("tty error\n");
  struct termios term = saved_term;
  term.c_lflag &= ~(ECHO | ICANON);
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;
  if (tcsetattr (0, TCSADRAIN, &term) == -1)
    errors_fatal ("tty error\n");

  if (tgetent (term_buffer, term_name) < 1)
    errors_fatal ("terminal type error\n");
}

