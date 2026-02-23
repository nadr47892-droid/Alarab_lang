#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "محرك_الاشارات.h"
#include "المنفذ.h"
#include "محرك_المتغيرات.h"
#include "المحرك_الرياضي.h"

// =====================================
// دالة لطباعة النصوص (نظيفة وبسيطة)
// =====================================
void rt_print(const char *text)
{
    if (!text || text[0] == '\0')
        return;

    printf("%s\n", text);
}

// =====================================
// دالة لتنظيف الشاشة
// =====================================
void rt_clear(void)
{
    printf("\033[2J\033[H");
}

// =====================================
// التنفيذ الرئيسي
// =====================================
int runtime_execute(Command cmd)
{
    switch (cmd.type)
    {

        // ===============================
        // طباعة
        // ===============================
        case CMD_PRINT:
            for (int i = 0; i < cmd.repeat; i++)
                rt_print(cmd.argument);
            break;

        // ===============================
        // تنظيف الشاشة
        // ===============================
        case CMD_CLEAR:
            for (int i = 0; i < cmd.repeat; i++)
                rt_clear();
            break;

        // ===============================
        // تنفيذ تعبير رياضي
        // ===============================
        case CMD_EXEC:
        {
            double value;

            if (math_eval(cmd.argument, &value) == 0)
                printf("الناتج = %g\n", value);
            else
                printf("❌ خطأ في التعبير الرياضي\n");

            break;
        }

        // ===============================
        // إسناد متغير
        // ===============================
        case CMD_ASSIGN:
        {
            char name[32];
            double val;

            char *eq = strchr(cmd.argument, '=');

            if (eq)
            {
                *eq = '\0';

                sscanf(cmd.argument, "%31s", name);

                if (math_eval(eq + 1, &val) == 0)
                {
                    var_set(name, val);
                    printf("تم حفظ %s = %g\n", name, val);
                }
                else
                {
                    printf("❌ خطأ في التعبير بعد علامة =\n");
                }
            }

            break;
        }

        // ===============================
        // حل معادلة
        // ===============================
        case CMD_SOLVE:
        {
            double value;
            char solved_var[32];

            int r = solve_equation(cmd.argument, &value, solved_var);

            if (r == 0)
            {
                printf("✅ النتيجة المكتشفة: %s = %g\n", solved_var, value);
                var_set(solved_var, value);
            }
            else if (r == -3)
            {
                printf("❌ يجب أن تحتوي المعادلة على مجهول واحد فقط\n");
            }
            else if (r == -4)
            {
                printf("❌ لا يوجد حل ضمن المجال المحدد\n");
            }
            else
            {
                printf("❌ فشل في تحليل المعادلة\n");
            }

            break;
        }

        // ===============================
        // تنفيذ أمر نظام
        // ===============================
        case CMD_EXEC_SYSTEM:
            execute_system_command(cmd.argument);
            break;

        // ===============================
        // خروج
        // ===============================
        case CMD_EXIT:
            return 1;

        // ===============================
        // غير معروف
        // ===============================
        case CMD_UNKNOWN:
        default:
            printf("❌ أمر غير معروف\n");
            break;
    }

    return 0;
}

