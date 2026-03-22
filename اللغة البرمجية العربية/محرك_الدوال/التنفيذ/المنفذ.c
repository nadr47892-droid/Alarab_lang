#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "المنفذ.h"
#include "محرك_الدالة.h"
#include "محرك_الاشارات.h"
#include "المحرك_الرياضي.h"
#include "مفسر_العمليات.h"

void execute_system_command(const char *cmd)
{
    if (!cmd || cmd[0] == '\0')
    {
        printf("خطأ: لا يوجد أمر للتنفيذ\n");
        return;
    }

    if (system(cmd) == -1)
        printf("فشل تنفيذ الأمر\n");
}

/* ===================================== */
/* تنفيذ كتلة أو جسم دالة                */
/* ===================================== */

static int execute_block(const char *body)
{
    char temp[4096];
    strncpy(temp, body, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';


    /* اصلاح }والا{ */

    char fixed[4096] = "";
    char *p = temp;

    while (*p)
    {
        if (strncmp(p, "}والا{", 7) == 0)
        {
            strcat(fixed, "}\nوالا{\n");
            p += 7;
        }
        else
        {
            size_t len = strlen(fixed);
            if (len < sizeof(fixed) - 1)
            {
                fixed[len] = *p;
                fixed[len+1] = '\0';
        }
            p++;
        }
    }

    strcpy(temp, fixed);

        char *lines[512];
        int total = 0;

    char *token = strtok(temp, "\n");

    while (token && total < 512)
    {
        lines[total++] = token;
        token = strtok(NULL, "\n");
    }

    for (int i = 0; i < total; i++)
    {
        Command cmd = parse_line(lines[i]);

        if (cmd.type == CMD_UNKNOWN)
            continue;

        /* ========================= */
        /* تعريف دالة داخل كتلة     */
        /* ========================= */

        if (cmd.type == CMD_FUNCTION_DEF)
        {
            char body2[4096] = "";
            int brace = 1;

            i++;

            while (i < total)
            {
                if (strchr(lines[i], '{')) brace++;
                if (strchr(lines[i], '}')) brace--;

                if (brace == 0)
                    break;

                strcat(body2, lines[i]);
                strcat(body2, "\n");

                i++;
            }

            function_store(cmd.name, body2);
            continue;
        }
        /* ========================= */
        /* تنفيذ اذا + والا اذا + والا */
        /* ========================= */

        if (cmd.type == CMD_IF_DEF)
        {
            char bodies[10][4096];
            char conditions[10][256];

            int count = 0;

            // =========================
            // أول if
            // =========================

            strcpy(conditions[count], cmd.argument);
            bodies[count][0] = '\0';

            int brace = 1;
            i++;

            while (i < total)
            {
                if (strchr(lines[i], '{')) brace++;
                if (strchr(lines[i], '}')) brace--;

                if (brace == 0)
                    break;

                strcat(bodies[count], lines[i]);
                strcat(bodies[count], "\n");
                i++;
            }

            count++;

            // =========================
            // قراءة else if و else
            // =========================

            while (i + 1 < total)
            {
                Command next = parse_line(lines[i + 1]);

                // ----------- والا اذا -----------
                if (next.type == CMD_ELSE_IF)
                {
                    i += 2;

                    strcpy(conditions[count], next.argument);
                    bodies[count][0] = '\0';

                    brace = 1;

                    while (i < total)
                    {
                        if (strchr(lines[i], '{')) brace++;
                        if (strchr(lines[i], '}')) brace--;

                        if (brace == 0)
                            break;

                        strcat(bodies[count], lines[i]);
                        strcat(bodies[count], "\n");
                        i++;
                    }

                    count++;
                }
                // ----------- والا -----------
                else if (next.type == CMD_ELSE)
                {
                    i += 2;

                    strcpy(conditions[count], "1"); // شرط دائم
                    bodies[count][0] = '\0';

                    brace = 1;

                    while (i < total)
                    {
                        if (strchr(lines[i], '{')) brace++;
                        if (strchr(lines[i], '}')) brace--;

                        if (brace == 0)
                            break;

                        strcat(bodies[count], lines[i]);
                        strcat(bodies[count], "\n");
                        i++;
                    }

                    count++;
                    break;
                }
                else
                {
                    break;
                }
            }

            // =========================
            // تنفيذ أول شرط صحيح
            // =========================

            for (int j = 0; j < count; j++)
            {
                double result = 0;

                if (math_eval(conditions[j], &result) == 0 && result != 0)
                {
                    execute_block(bodies[j]);
                    break;
                }
            }

            continue;
        }
        /* ========================= */
        /* تنفيذ الأمر              */
        /* ========================= */

        if (execute_command(cmd))
            return 1;
    }

    return 0;
}

/* ===================================== */
/* التنفيذ الرئيسي للأوامر               */
/* ===================================== */

int execute_command(Command cmd)
{

    /* ========================= */
    /* استدعاء دالة             */
    /* ========================= */

    if (cmd.type == CMD_FUNCTION_CALL)
    {
        const char *body = function_get(cmd.name);

        if (!body)
        {
            printf("خطأ: الدالة '%s' غير موجودة\n", cmd.name);
            return 0;
        }

        return execute_block(body);
    }


    /* ========================= */
/* تحميل ملف خارجي          */
/* ========================= */

    if (cmd.type == CMD_IMPORT)
    {
        FILE *f = fopen(cmd.argument, "r");

        if (!f)
        {
            printf("❌ لا يمكن فتح الملف: %s\n", cmd.argument);
            return 0;
        }

        char buffer[4096] = "";
        char line[512];

        while (fgets(line, sizeof(line), f))
        {
            strcat(buffer, line);
        }

        fclose(f);

    // تنفيذ محتوى الملف    
        return execute_block(buffer);
    }

    /* ========================= */
    /* تنفيذ أمر نظام           */
    /* ========================= */

    if (cmd.type == CMD_EXEC_SYSTEM)
    {
        execute_system_command(cmd.argument);
        return 0;
    }

    /* ========================= */
    /* تنفيذ الأوامر العادية    */
    /* ========================= */

    if (runtime_execute(cmd))
        return 1;

    return 0;
}
