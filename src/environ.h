#ifndef PHILLYSHELL_ENVIRON_H
#define PHILLYSHELL_ENVIRON_H

#include "shell.h"

#define PATH "PATH"

sh_err parse_environ (Shell *sh, char **environ);
sh_err parse_path (Shell *sh);

#endif // PHILLYSHELL_ENVIRON_H
