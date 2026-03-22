#include "محرك_المقارنات.h"
#include "محرك_العمليات_الحسابية_الاساسية.h"

extern const char *p;

int basic_expression(double *v);

int parse_comparison(double left, double *result)
{
    if (*p != '<' && *p != '>' && *p != '=')
        return 1; /* لا يوجد مقارنة */

    char op1 = *p++;
    char op2 = 0;

    if (*p == '=')
        op2 = *p++;

    double right;

    if (basic_expression(&right) != 0)
        return -1;

    if (op1 == '<' && op2 == '=')
        *result = (left <= right);
    else if (op1 == '>' && op2 == '=')
        *result = (left >= right);
    else if (op1 == '<')
        *result = (left < right);
    else if (op1 == '>')
        *result = (left > right);
    else if (op1 == '=' && op2 == '=')
        *result = (left == right);
    else
        return -1;

    return 0;
}
