#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"

void chrdump(char str)
{
    if (str < ' ' || str > '~')
        printf("\\%#x", str);
    else
        printf("%c", str);
}

void strdump(char *str)
{
    while (*str)
        chrdump(*str++);
}

int eat_string(char **req, char *eat)
{
    if (strncmp(*req, eat, strlen(eat)) == 0)
    {
        *req += strlen(eat);
        return 1;
    }
    return 0;
}

int eat_char(char **req, char eat)
{
    if (**req == eat)
    {
        *req += 1;
        return 1;
    }
    return 0;
}

int eat_list(char **req, char *list)
{
    if (index(list, **req) && **req)
    {
        *req += 1;
        return 1;
    }
    return 0;
}

int eat_range(char **req, char start, char end)
{
    if (**req >= start && **req <= end && **req)
    {
        *req += 1;
        return 1;
    }
    return 0;
}
