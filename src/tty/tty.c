#include "tty.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

struct termios tty_default;
struct termios tty_fsh;

//sh_ecode
//tty_setraw (t_tty *tty)
//{
//  if (!isatty (STDIN_FILENO) && !isatty (STDOUT_FILENO))
//    {
//      return SH_ERR;
//    }
//  if (tcgetattr (STDIN_FILENO, &tty->curr) < 0)
//    {
//      perror ("fsh");
//      return SH_ERR;
//    }
//  tty->save = tty->curr;
//  tty->curr.c_lflag &= ~(ECHO | ICANON);
//  tty->curr.c_cc[VMIN] = 1;
//  tty->curr.c_cc[VTIME] = 0;
//  if (tcgetattr (STDIN_FILENO, &tty->curr) < 0)
//    {
//      perror ("fsh");
//      tcsetattr (STDIN_FILENO, TCSAFLUSH, &tty->save);
//      return SH_ERR;
//    }
//  if ((tty->curr.c_lflag & (ECHO | ICANON)) || tty->curr.c_cc[VMIN] != 1
//      || tty->curr.c_cc[VTIME] != 0)
//    {
//      tcsetattr (STDIN_FILENO, TCSAFLUSH, &tty->save);
//      return SH_ERR;
//    }
//}

sh_ecode tty_init() {
      if (tcgetattr (STDIN_FILENO, &tty_default) < 0)
    {
      perror ("fsh");
      return SH_ERR;
    }
    if (tcgetattr (STDIN_FILENO, &tty_fsh) < 0)
    {
        perror ("fsh");
        return SH_ERR;
    }
    return SH_OK;
}