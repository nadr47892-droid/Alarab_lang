#include <math.h>
#include <string.h>
#include "محرك_الجذور.h"

/* سيتم جلب هذه من المحرك الرئيسي */

extern const char *p;

void skip_spaces();
int basic_expression(double *v);


/* =========================================================
   الجذور
========================================================= */

int parse_root(double *v)
{
    skip_spaces();

    if (strncmp(p, "جذر", 6) != 0)
        return 1;   /* ليس جذر */

    p += 6;

    if (*p++ != '(')
        return -1;

    double x;

    if (basic_expression(&x) != 0)
        return -1;

    skip_spaces();

    /* جذر نوني */

    if (*p == ',') {

        p++;

        double n;

        if (basic_expression(&n) != 0 || n == 0)
            return -1;

        if (*p++ != ')')
            return -1;

        *v = pow(x, 1.0 / n);
        return 0;
    }

    /* جذر تربيعي */

    if (*p++ != ')')
        return -1;

    if (x < 0)
        return -1;

    *v = sqrt(x);

    return 0;
}
