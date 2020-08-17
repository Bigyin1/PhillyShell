#ifndef PHILLYSHELL_SHELL_H
#define PHILLYSHELL_SHELL_H

typedef struct s_cfg
{
  char *prompt;

} t_config;

typedef struct s_shell
{
  t_config *cfg;
} Shell;

#endif // PHILLYSHELL_SHELL_H
