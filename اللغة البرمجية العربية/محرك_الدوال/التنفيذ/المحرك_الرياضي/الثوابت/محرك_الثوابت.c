#include <math.h>
#include <string.h>
#include "محرك_الثوابت.h"

/* المؤشر القادم من المحرك الرئيسي */

extern const char *p;

/* =========================================================
   الثوابت الرياضية
========================================================= */

int parse_constant(double *v)
{
    /* pi */

    if (strncmp(p, "pi", 2) == 0) {
        p += 2;
        *v = M_PI;
        return 0;
    }

    /* π */

    if (strncmp(p, "π", strlen("π")) == 0) {
        p += strlen("π");
        *v = M_PI;
        return 0;
    }

    /* فاي */

    if (strncmp(p, "فاي", strlen("فاي")) == 0) {
        p += strlen("فاي");
        *v = M_PI;
        return 0;
    }

    return 1;   /* ليس ثابت */
}
