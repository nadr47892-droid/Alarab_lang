#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "محرك_الاشارات.h"
#include "المنفذ.h"
#include "محرك_المتغيرات.h"
#include "المحرك_الرياضي.h"

// =====================================
// دالة لطباعة النصوص
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
        // ===============================
        // طباعة
        // ===============================
        case CMD_PRINT:
        {
            for (int i = 0; i < cmd.repeat; i++)
            {
                char buffer[256];
                strncpy(buffer, cmd.argument, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';

                // تقسيم العناصر داخل ()
                char *token = strtok(buffer, ",");

                while (token)
                {
                    // حذف المسافات من البداية
                    while (*token == ' ')
                        token++;

                    // حذف المسافات من النهاية
                    int len = strlen(token);
                    while (len > 0 && token[len - 1] == ' ')
                    {
                        token[len - 1] = '\0';
                        len--;
                    }

                    // ===============================
                    // إذا كان نص
                    // ===============================
                    if (len >= 2 && token[0] == '"' && token[len - 1] == '"')
                    {
                        token[len - 1] = '\0';
                        printf("%s ", token + 1);
                    }
                    // ===============================
                    // إذا كان متغير
                    // ===============================
                    else
                    {
                        double value;
                        int res = var_get(token, &value);

                        if (res == 1)
                        {
                            printf("%g ", value);
                        }
                        else if (res == -1)
                        {
                            printf("❌ المتغير %s ليس له قيمة ", token);
                        }
                        else
                        {
                            printf("❌ المتغير %s غير موجود ", token);
                        }
                    }

                    token = strtok(NULL, ",");
                }

                printf("\n");
            }

            break;
        }

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
            int ret = math_eval(cmd.argument, &value);

            if (ret == 0)   // حساب double
            {
                if (fabs(value - (long long)value) < 1e-9)
                    printf("الناتج = %lld\n", (long long)value);
                else
                    printf("الناتج = %.10f\n", value);
            }
            else if (ret == 2)
            {
                // BigInt تم طباعته داخل المحرك
                // لا نفعل شيء هنا
            }
            else
            {
                printf("❌ خطأ في التعبير الرياضي\n");
            }

            break;
        }

        case CMD_INPUT:
        {
            double value;

            printf("ادخل قيمة %s: ", cmd.argument);

        if (scanf("%lf", &value) != 1)
        {
            printf("❌ إدخال غير صالح\n");

            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            break;
        }

            getchar(); // تنظيف السطر

            var_set(cmd.argument, value);

            printf("تم حفظ %s = %.2f\n", cmd.argument, value);

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

                char *value_str = eq + 1;

                while (*value_str == ' ')
                    value_str++;

                // ===============================
                // متغير بدون قيمة
                // ===============================
                if (strcmp(value_str, "؟") == 0)
                {
                    var_set(name, NAN);
                    printf("تم تعريف %s كمتغير بدون قيمة\n", name);
                    break;
                }

                // ===============================
                // إسناد عادي
                // ===============================
                if (math_eval(value_str, &val) == 0)
                {
                    var_set(name, val);

                    if (fabs(val - (long long)val) < 1e-9)
                        printf("تم حفظ %s = %lld\n", name, (long long)val);
                    else
                        printf("تم حفظ %s = %.10f\n", name, val);
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
                if (fabs(value - (long long)value) < 1e-9)
                    printf("✅ النتيجة المكتشفة: %s = %lld\n", solved_var, (long long)value);
                else
                    printf("✅ النتيجة المكتشفة: %s = %.10f\n", solved_var, value);

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
