#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "المحرك_الرياضي.h"
#include "محرك_الدوال_المثلثية.h"



static void skip_spaces(const char **p)
{
    while (**p == ' ')
        (*p)++;
}

int trig_parse(const char **ptr, double *v)
{
    const char *p = *ptr;

    skip_spaces(&p);

    const char *name = NULL;

    if (strncmp(p, "sin", 3) == 0) name = "sin";
    else if (strncmp(p, "cos", 3) == 0) name = "cos";
    else if (strncmp(p, "tan", 3) == 0) name = "tan";
    else return -1;

    p += strlen(name);

    int exp = 1;

    skip_spaces(&p);

    /* دعم الأس */
    if (*p == '^')
    {
        p++;

        char num[32];
        int i = 0;

        while ((*p >= '0' && *p <= '9') && i < 30)
            num[i++] = *p++;

        num[i] = '\0';

        if (i == 0)
            return -1;

        exp = atoi(num);
    }

    skip_spaces(&p);

    if (*p++ != '(')
        return -1;

    const char *start = p;

    int level = 1;

    while (*p && level)
    {
        if (*p == '(') level++;
        if (*p == ')') level--;
        p++;
    }

    if (level != 0)
        return -1;

    size_t len = (p - 1) - start;

    char expr[128];

    if (len >= sizeof(expr))
        return -1;

    strncpy(expr, start, len);
    expr[len] = '\0';

    double arg;

    if (math_eval(expr, &arg) != 0)
        return -1;

    double rad = arg * M_PI / 180.0;

    double base;

    if (strcmp(name, "sin") == 0)
        base = sin(rad);
    else if (strcmp(name, "cos") == 0)
        base = cos(rad);
    else
        base = tan(rad);

    *v = pow(base, exp);

    *ptr = p;

    return 0;
}
