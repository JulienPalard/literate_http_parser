#ifndef __PARSER_H__
#define __PARSER_H__

#include <strings.h>

void chrdump(char str);
void strdump(char *str);
int eat_string(char **req, char *eat);
int eat_char(char **req, char eat);
int eat_list(char **req, char *list);
int eat_range(char **req, char start, char end);

typedef void (*callback)(char *rule, char *ptr, int len,
                         void *user_data);

#define CONCAT(a, b) a ## b

#define DECLARE_RULE(name) \
    char *rule_ ## name(char **req, callback out, void *user_data);

#define RULE(name, code)                                                \
    char *rule_ ## name(char **req, callback out, void *user_data) \
    {                                                                   \
        char *rollback = *req;                                          \
                                                                        \
        code;                                                           \
        if (*req != rollback)                                           \
            out(#name, rollback, *req - rollback, user_data);           \
        return *req;                                                    \
    }

#define PARSER_FAIL                                                     \
    {                                                                   \
        *req = rollback;                                                \
        return NULL;                                                    \
    }

#define PARSE_TRY(st)                                                   \
    {                                                                   \
        char *rollback = *req;                                          \
        if (!(st))                                                      \
        {                                                               \
            *req = rollback;                                            \
            run = 0;                                                    \
        }                                                               \
    }

#define PARSE_OPTIONAL(st)                                              \
    {                                                                   \
        char *rollback = *req;                                          \
        if (!(st))                                                      \
            *req = rollback;                                            \
    }

#define STRING(a)   eat_string(req, a)
#define CHAR(a)     eat_char(req, a)
#define RANGE(a, b) eat_range(req, a, b)
#define LIST(a)     eat_list(req, a)
#define CALL(a)     rule_ ## a(req, out, user_data)
#define OPTIONAL(rules) {PARSE_OPTIONAL(rules);}
#define MANY(rules)     {int run = 1; while (run) PARSE_TRY(rules)}
#define ONE(a)      if (!(a)) PARSER_FAIL
#define NOT(a)      if (a) PARSER_FAIL
#define AT_LEAST_ONE(a) if (!(a)) PARSER_FAIL MANY(a)

void debug(int level, char *format, ...);

#endif
