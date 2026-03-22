#include "محرك_العمليات_الحسابية_الاساسية.h"
#include "المحرك_الرياضي.h"

/* سيتم جلب هذه من المحرك الرئيسي */
extern const char *p;

int term(double *v);
void skip_spaces();
int power(double *v);
int starts_factor(char c);

/* الجمع والطرح */

int basic_expression(double *v)
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
/*
   الضرب والقسمة
========================================================= */
int term(double *v)
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

