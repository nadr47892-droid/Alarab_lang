#include <math.h>
#include "محرك_الاسس.h"

/* سيتم جلب هذه من المحرك الرئيسي */

extern const char *p;

void skip_spaces();
int factor(double *v);

/* =========================================================
   الأس ^
========================================================= */
int power(double *v)
{
    if (factor(v) != 0)
        return -1;

    skip_spaces();

    /* دعم الأس المتسلسل Right Associative */
    while (*p == '^') {

        p++;

        double exponent;

        /* دعم الأس السالب أو المركب */
        if (factor(&exponent) != 0)
            return -1;

        /* منع 0^سالب */
        if (*v == 0 && exponent < 0)
            return -2;

        *v = pow(*v, exponent);

        skip_spaces();
    }

    return 0;
}

