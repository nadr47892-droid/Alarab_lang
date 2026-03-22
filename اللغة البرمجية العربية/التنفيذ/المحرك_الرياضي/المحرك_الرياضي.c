#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "محرك_الجذور.h"
#include "محرك_الثوابت.h"
#include "محرك_الاسس.h"
#include "المحرك_الرياضي.h"
#include "محرك_المتغيرات.h"
#include "محرك_المقارنات.h"
#include "محرك_الدوال_المثلثية.h"
#include "محرك_العمليات_الحسابية_الاساسية.h"


/* =========================
   مؤشر داخلي
========================= */
const char *p;

/* تصريحات داخلية */
int factor(double *v);
static int parse_function(double *v);
int starts_factor(char c);
static int find_unknown(const char *exp, char *unknown_out);

/* =========================================================
   دالة تقييم مع حقن قيمة للمجهول
========================================================= */
static int eval_with_unknown(
    const char *exp,
    const char *unknown,
    double val,
    double *res
) {
    var_set(unknown, val);
    return math_eval(exp, res);
}

/* =========================================================
   خوارزمية حل المعادلات (Bisection Method محسنة)
========================================================= */
int solve_equation(const char *expression_text,
                   double *result,
                   char *solved_var)
{
    char unknown[32];

    if (!find_unknown(expression_text, unknown))
        return -3;

    double start = -1000.0;
    double end   =  1000.0;
    double step  = 1.0;

    double x1 = start;
    double f1;

    if (eval_with_unknown(expression_text, unknown, x1, &f1) != 0)
        return -2;

    /* ===== مسح المجال لاكتشاف تغير الإشارة ===== */
    for (double x2 = start + step; x2 <= end; x2 += step) {

        double f2;

        if (eval_with_unknown(expression_text, unknown, x2, &f2) != 0)
            return -2;

        if (f1 * f2 <= 0) {

            /* وجدنا مقطع يحتوي جذر → نطبق Bisection داخله */

            double low = x1;
            double high = x2;
            double mid, fmid;

            for (int i = 0; i < 100; i++) {

                mid = (low + high) / 2.0;

                if (eval_with_unknown(expression_text, unknown, mid, &fmid) != 0)
                    return -2;

                if (fabs(fmid) < 1e-9)
                    break;

                if (f1 * fmid < 0) {
                    high = mid;
                } else {
                    low = mid;
                    f1 = fmid;
                }
            }

            *result = mid;
            var_set(unknown, mid);

            snprintf(solved_var, 32, "%s", unknown);

            return 0;
        }

        x1 = x2;
        f1 = f2;
    }

    return -4;  // لم نجد جذر
}


/* =========================================================
   اكتشاف المجهول
========================================================= */
static int find_unknown(const char *exp, char *unknown_out)
{
    char found[32] = "";
    int found_once = 0;

    for (int i = 0; exp[i] != '\0'; i++) {

        if (isalpha(exp[i]) || (unsigned char)exp[i] >= 0x80) {

            char name[32];
            int j = 0;

            while ((isalpha(exp[i]) || (unsigned char)exp[i] >= 0x80)
                   && j < 31)
            {
                name[j++] = exp[i++];
            }

            name[j] = '\0';
            i--;

            double tmp;

            if (!var_get(name, &tmp)) {

                if (!found_once) {
                    snprintf(found, sizeof(found), "%s", name);
                    found_once = 1;
                }
                else if (strcmp(found, name) != 0) {
                    return 0;   /* أكثر من مجهول */
                }
            }
        }
    }

    if (found_once) {
        snprintf(unknown_out, 32, "%s", found);
        return 1;
    }

    return 0;
}


/* =========================================================
   أدوات مساعدة
========================================================= */

void skip_spaces() {
    while (*p == ' ')
        p++;
}

int starts_factor(char c) {
    return isdigit(c) || c == '(' || c == '.' ||
           isalpha(c) || (unsigned char)c >= 0x80;
}

/* =========================================================
   قراءة رقم
========================================================= */
static int read_number(double *v)
{
    skip_spaces();

    int sign = 1;

    if (*p == '-') {
        sign = -1;
        p++;
    }

    if (!isdigit(*p) && *p != '.')
        return -1;

    double value = 0.0;

    while (isdigit(*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    if (*p == '.') {
        p++;
        double base = 0.1;
        while (isdigit(*p)) {
            value += (*p - '0') * base;
            base *= 0.1;
            p++;
        }
    }

    *v = value * sign;
    return 0;
}

/* =========================================================
   factor
========================================================= */
int factor(double *v)
{
    skip_spaces();

    /* الثوابت */

    if (parse_constant(v) == 0)
        return 0;

    /* السالب */

    if (*p == '-') {
        p++;
        if (factor(v) != 0)
            return -1;
        *v = -*v;
        return 0;
    }

    /* الأقواس */

    if (*p == '(') {
        p++;

        if (basic_expression(v) != 0)
            return -1;

        skip_spaces();

        if (*p != ')')
            return -1;

        p++;
        return 0;
    }

    /* المتغيرات */

    if ((unsigned char)*p >= 0x80 || isalpha(*p)) {

        char var_name[32];
        int i = 0;

        while (((unsigned char)*p >= 0x80 || isalnum(*p)) && i < 31)
            var_name[i++] = *p++;

        var_name[i] = '\0';

        if (var_get(var_name, v))
            return 0;

        p -= i;
    }

    /* الدوال */

    if (parse_function(v) == 0)
        return 0;

    /* الأرقام */

    return read_number(v);
}





/* =========================================================
   الدوال الرياضية
========================================================= */
static int parse_function(double *v) {
    skip_spaces();

    if (parse_root(v) == 0)
        return 0;


    /* ===== دوال مثلثية ===== */

    if (trig_parse(&p, v) == 0)
        return 0;

    return -1;
}



/* =========================================================
   الواجهة العامة
========================================================= */

/* التحقق أن "و" منطقية وليست داخل كلمة */

/* التحقق أن "او" منطقية */
static int is_logic_or(const char *s, int i)
{
    char before = (i > 0) ? s[i - 1] : ' ';
    char after  = s[i + 4];

    if (isalpha(before) || (unsigned char)before >= 0x80) return 0;
    if (isalpha(after)  || (unsigned char)after  >= 0x80) return 0;

    return 1;
}

int math_eval(const char *expression_text,
              double *result)
{
    /* =========================
       دعم AND (و) بدون مسافات
    ========================= */

{
    const char *start = expression_text;
    int has_comma = 0;

    for (int i = 0; expression_text[i] != '\0'; i++)
    {
        if (expression_text[i] == ',')
        {
            has_comma = 1;
            break;
        }
    }

    if (has_comma)
    {

        while (1)
        {
            const char *comma = strchr(start, ',');

            char part[256];

            if (comma)
            {
                int len = comma - start;
                if (len >= 255) len = 255;

                strncpy(part, start, len);
                part[len] = '\0';
            }
            else
            {
                strncpy(part, start, 255);
                part[255] = '\0';
            }

            /* حذف المسافات من البداية */
            char *p = part;
            while (*p == ' ') p++;

            /* حذف المسافات من النهاية */
            char *end = p + strlen(p) - 1;
            while (end > p && *end == ' ')
            {
                *end = '\0';
                end--;
            }

            double val = 0;

            if (math_eval(p, &val) != 0)
                return -1;

            if (val == 0)
            {
                *result = 0;
                return 0;
            }

            if (!comma)
                break;

            start = comma + 1;
        }

        *result = 1;
        return 0;
    }
    }

    /* =========================
       دعم OR (او) بدون مسافات
    ========================= */
    for (int i = 0; expression_text[i] != '\0'; i++)
    {
        if (strncmp(&expression_text[i], "او", 4) == 0 &&
            is_logic_or(expression_text, i))
        {
            char left[256];
            char right[256];

            strncpy(left, expression_text, i);
            left[i] = '\0';

            strcpy(right, &expression_text[i + 4]);

            double l = 0, r = 0;

            if (math_eval(left, &l) != 0) return -1;
            if (math_eval(right, &r) != 0) return -1;

            *result = (l != 0 || r != 0);
            return 0;
        }
    }

    /* =========================
       الكود القديم
    ========================= */

    p = expression_text;

    double left;

    if (basic_expression(&left) != 0)
        return -1;

    skip_spaces();

    double cmp;
    int r = parse_comparison(left, &cmp);

    if (r == 0)
    {
        *result = cmp;
        return 0;
    }
    else if (r < 0)
    {
        return -1;
    }

    *result = left;

    skip_spaces();

    return (*p == '\0') ? 0 : -1;
}
