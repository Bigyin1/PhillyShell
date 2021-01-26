#include "tty.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termcap.h>
#include <unistd.h>

#define LF '\n'
#define DEL 127
#define ESC 27
#define LP '['
#define LEFT 'D'
#define UP 'W'
#define DOWN 'S'
#define RIGHT 'C'

typedef struct
{
  String *cmd;
  size_t cursor_x;
  char key[3];
} tty_line;

int
my_out (int c)
{
  write (1, &c, 1);
  return 1;
}

void
read_key (tty_line *l)
{
  memset (l->key, 0, 3);
  if (read (STDIN_FILENO, l->key, 3) == -1)
    {
      perror ("fsh: read error:");
      exit (EXIT_FAILURE);
    }
}

void
add_char (tty_line *l, char c)
{
  string_insert (l->cmd, l->cursor_x++, c);
  if (l->cursor_x == string_len (l->cmd))
    {
      write (STDOUT_FILENO, &c, 1);
      return;
    }
  tputs (tgetstr ("sc", NULL), 1, &my_out);
  char *left = tgetstr ("le", NULL);
  for (int i = 0; i < l->cursor_x - 1; ++i)
    write (STDOUT_FILENO, left, strlen (left));

  write (STDOUT_FILENO, string_to_c_str (l->cmd), string_len (l->cmd));
  tputs (tgetstr ("rc", NULL), 1, &my_out);
  tputs (tgetstr ("nd", NULL), 1, &my_out);
}

void
del_char (tty_line *l)
{
  if (l->cursor_x == 0)
    return;
  string_erase_from_idx (l->cmd, --l->cursor_x);
  tputs (tgetstr ("le", NULL), 1, my_out);
  tputs (tgetstr ("dm", NULL), 1, my_out);
  tputs (tgetstr ("dc", NULL), 1, my_out);
  tputs (tgetstr ("ed", NULL), 1, my_out);
}

void
process_escapes (tty_line *l)
{
  if (l->key[1] != LP)
    return;

  if (l->key[2] == LEFT)
    {
      if (l->cursor_x == 0)
        return;
      tputs (tgetstr ("le", NULL), 1, my_out);
      l->cursor_x--;
      return;
    }
  if (l->key[2] == RIGHT)
    {
      if (l->cursor_x == string_len (l->cmd))
        return;
      tputs (tgetstr ("nd", NULL), 1, my_out);
      l->cursor_x++;
      return;
    }
}

bool
process_input (tty_line *l)
{
  read_key (l);
  if (l->key[0] == LF)
    {
      putchar (LF);
      return true;
    }
  if (l->key[0] >= 32 && l->key[0] < 127)
    {
      add_char (l, l->key[0]);
      if (l->key[1] >= 32 && l->key[1] < 127)
        add_char (l, l->key[1]);
      if (l->key[2] >= 32 && l->key[2] < 127)
        add_char (l, l->key[2]);
      return false;
    }
  if (l->key[0] == DEL)
    {
      del_char (l);
      return false;
    }
  if (l->key[0] == ESC)
    {
      process_escapes (l);
      return false;
    }
  return false;
}

String *
tty_readline ()
{
  tty_line l = { 0 };
  string_new (&l.cmd, 0, 0);

  write (STDOUT_FILENO, "$> ", 3);

  while (!process_input (&l))
    {
    };

  return l.cmd;
}