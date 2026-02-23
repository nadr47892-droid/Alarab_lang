#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "محرك_المتغيرات.h"
#include "المحرك_الرياضي.h"

/* =========================
   مؤشر داخلي
========================= */
static const char *p;

/* تصريحات داخلية */
static int expression(double *v);
static int term(double *v);
static int factor(double *v);
static int parse_function(double *v);
static int power(double *v);
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

            strncpy(solved_var, unknown, 31);
            solved_var[31] = '\0';

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
                    strncpy(found, name, 31);
                    found[31] = '\0';
                    found_once = 1;
                }
                else if (strcmp(found, name) != 0) {
                    return 0; // أكثر من مجهول مختلف
                }
            }
        }
    }

    if (found_once) {
        strncpy(unknown_out, found, 31);
        unknown_out[31] = '\0';
        return 1;
    }

    return 0;
}


/* =========================================================
   أدوات مساعدة
========================================================= */

static void skip_spaces() {
    while (*p == ' ')
        p++;
}

static int starts_factor(char c) {
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
static int factor(double *v)
{
    skip_spaces();

    /* ثوابت */
    if (strncmp(p, "pi", 2) == 0) {
        p += 2;
        *v = M_PI;
        return 0;
    }

    if (strncmp(p, "π", strlen("π")) == 0) {
        p += strlen("π");
        *v = M_PI;
        return 0;
    }

    if (strncmp(p, "فاي", strlen("فاي")) == 0) {
        p += strlen("فاي");
        *v = M_PI;
        return 0;
    }

    if (*p == '-') {
        p++;
        if (factor(v) != 0)
            return -1;
        *v = -*v;
        return 0;
    }

    if (*p == '(') {
        p++;
        if (expression(v) != 0)
            return -1;

        skip_spaces();

        if (*p != ')')
            return -1;

        p++;
        return 0;
    }

    if ((unsigned char)*p >= 0x80 || isalpha(*p)) {

        char var_name[32];
        int i = 0;

        while (((unsigned char)*p >= 0x80 || isalnum(*p))
               && i < 31)
        {
            var_name[i++] = *p++;
        }

        var_name[i] = '\0';

        if (var_get(var_name, v))
            return 0;

        p -= i;
    }

    if (parse_function(v) == 0)
        return 0;

    return read_number(v);
}

/* =========================================================
   الأس ^
========================================================= */
static int power(double *v)
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


/* =========================================================
   الضرب والقسمة
========================================================= */
static int term(double *v)
{
    if (power(v) != 0)
        return -1;

    while (1) {

        skip_spaces();

        if (*p == '*' || *p == '/') {

            char op = *p++;
            double right;

            if (power(&right) != 0)
                return -1;

            if (op == '*')
                *v *= right;
            else {
                if (right == 0)
                    return -2;
                *v /= right;
            }

            continue;
        }

        if (starts_factor(*p)) {

            double right;

            if (power(&right) != 0)
                return -1;

            *v *= right;
            continue;
        }

        break;
    }

    return 0;
}

/* =========================================================
   الجمع والطرح
========================================================= */
static int expression(double *v)
{
    if (term(v) != 0)
        return -1;

    while (1) {

        skip_spaces();

        if (*p != '+' && *p != '-')
            break;

        char op = *p++;
        double right;

        if (term(&right) != 0)
            return -1;

        if (op == '+')
            *v += right;
        else
            *v -= right;
    }

    return 0;
}

/* =========================================================
   الدوال الرياضية
========================================================= */
static int parse_function(double *v)
{
    skip_spaces();

    if (strncmp(p, "جذر", 6) == 0) {

        p += 6;

        if (*p++ != '(')
            return -1;

        double x;

        if (expression(&x) != 0)
            return -1;

        if (*p++ != ')')
            return -1;

        if (x < 0)
            return -1;

        *v = sqrt(x);
        return 0;
    }

    return -1;
}

/* =========================================================
   الواجهة العامة
========================================================= */
int math_eval(const char *expression_text,
              double *result)
{
    p = expression_text;

    if (expression(result) != 0)
        return -1;

    skip_spaces();

    return (*p == '\0') ? 0 : -1;
}

