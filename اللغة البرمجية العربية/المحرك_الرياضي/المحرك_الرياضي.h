#ifndef MATH_ENGINE_H
#define MATH_ENGINE_H

/* تقييم تعبير رياضي */
int math_eval(const char *expression, double *result);


/* حل المعادلات - الإضافة الجديدة */
int solve_equation(const char *expression_text,
                   double *result,
                   char *solved_var);


#endif

