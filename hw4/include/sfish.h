#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define CD_PATH_NEXIST "cd: path not exist\n"
#define HOME_NOT_INIT "cd: home not init\n"
#define REDIRECTION_SYNTAX_ERROR "Syntax error with redirection\n"
#define FILE_NO_EXIST_ERROR "File doesnt exist\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

#endif
