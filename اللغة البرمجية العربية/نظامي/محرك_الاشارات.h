#ifndef RUNTIME_H
#define RUNTIME_H
#include "مفسر_العمليات.h"
void rt_print(const char *text);   //تعريف الدالة اطبع
void rt_clear(void);              // تعريف الدالة نظف
int runtime_execute(Command cmd);  // ← ترجع 0 أو 1    //تعريف الدلة  نفذ 



#endif

