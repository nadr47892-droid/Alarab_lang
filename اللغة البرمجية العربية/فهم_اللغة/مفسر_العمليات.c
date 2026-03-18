#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "مفسر_العمليات.h"
#include "القاموس.h"
#include "محرك_الدالة.h"

static void trim(char *str)
{
    if (!str) return;

    int start = 0;

    while (isspace((unsigned char)str[start]))
        start++;

    if (start > 0)
        memmove(str, str + start, strlen(str + start) + 1);

    int end = strlen(str) - 1;

    while (end >= 0 && isspace((unsigned char)str[end]))
    {
        str[end] = '\0';
        end--;
    }
}

Command parse_line(const char *line)
{
    Command cmd;

    cmd.type = CMD_UNKNOWN;
    cmd.name[0] = '\0';
    cmd.argument[0] = '\0';
    cmd.repeat = 1;

    if (!line)
        return cmd;

    char clean[512];

    strncpy(clean, line, sizeof(clean) - 1);
    clean[sizeof(clean) - 1] = '\0';

    clean[strcspn(clean, "\n")] = '\0';

    trim(clean);

    if (strlen(clean) == 0)
        return cmd;

    /* تجاهل الأقواس */
    if (strcmp(clean, "{") == 0 || strcmp(clean, "}") == 0)
        return cmd;

    /* ================================
       كلمات ثابتة
    ================================= */

    if (strcmp(clean, كلمة_خروج) == 0)
    {
        cmd.type = CMD_EXIT;
        return cmd;
    }

    if (strcmp(clean, كلمة_نظف) == 0)
    {
        cmd.type = CMD_CLEAR;
        return cmd;
    }

    /* ================================
       تعريف دالة
    ================================= */

    if (strncmp(clean, "دالة(", 5) == 0)
    {
        char name[64];
        char extra[64];

        if (sscanf(clean, "دالة(%63[^)])%63s", name, extra) >= 1)
        {
        /* إذا وجد فاصلة بعد الدالة فهذا خطأ */ 
            if (strchr(clean, ','))
            {
                printf("خطأ: لا يمكن وضع شرط داخل تعريف الدالة\n");
                return cmd;
            }
    
            cmd.type = CMD_FUNCTION_DEF;
            strcpy(cmd.name, name);
            return cmd;
        }
    }
 //else if
    if (strncmp(clean, "والا اذا(", 8) == 0)
    {
        char condition[256];

        if (sscanf(clean, "والا اذا(%255[^)])", condition) == 1)
        {
            cmd.type = CMD_ELSE_IF;
            strcpy(cmd.argument, condition);
            return cmd;
        }
    }


    /* ================================
   والا (else)
================================ */

    if (strncmp(clean, "والا", 4) == 0)
    {
        cmd.type = CMD_ELSE;
        return cmd;
    }
    /* ================================
       تعريف شرط
    ================================= */

    if (strncmp(clean, "اذا(", 4) == 0)
    {
        char condition[256];


    /* اذا(شرط)*/

        if (sscanf(clean, "اذا(%255[^)])", condition) == 1)
        {
            cmd.type = CMD_IF_DEF;

            cmd.name[0] = '\0';   /* بدون اسم */
            strcpy(cmd.argument, condition);

            return cmd;
        }
    }

    /* ================================
       استدعاء دالة أو شرط
    ================================= */

    int len = strlen(clean);

    if (len > 2 && strcmp(clean + len - 2, "()") == 0)
    {
        int name_len = len - 2;

        if (name_len > 63)
            name_len = 63;

        snprintf(cmd.name, sizeof(cmd.name), "%.*s", name_len, clean);


        if (function_get(cmd.name))
        {
            cmd.type = CMD_FUNCTION_CALL;
            return cmd;
        }

        return cmd;
    }
    /* ================================

       5. حل المعادلات
    ================================= */

    if (strncmp(clean, "حل", 2) == 0) {

        char *first_equal = strchr(clean, '=');

        if (!first_equal)
            return cmd;

        char *eq_part = first_equal + 1;

        while (*eq_part == ' ')
            eq_part++;

        if (*eq_part == '\0')
            return cmd;

        char *second_equal = strchr(eq_part, '=');

        cmd.type = CMD_SOLVE;

        if (second_equal) {

            *second_equal = '\0';

            snprintf(cmd.argument,
                     sizeof(cmd.argument),
                     "(%s)-(%s)",
                     eq_part,
                     second_equal + 1);
        }
        else {

            snprintf(cmd.argument,
                     sizeof(cmd.argument),
                     "%s",
                     eq_part);
        }

        return cmd;
    }

    /* ================================
       6. اطبع
    ================================= */

    if (strncmp(clean, كلمة_اطبع, strlen(كلمة_اطبع)) == 0) {

        char *eq = strchr(clean, '=');

        if (!eq)
            return cmd;

        eq++;

        while (*eq == ' ')
            eq++;

        if (*eq != '(')
            return cmd;

        char *open = eq;
        char *close = strchr(open, ')');

        if (!close)
            return cmd;

        char text[256];

        size_t text_len = close - open - 1;

        if (text_len >= sizeof(text))
            text_len = sizeof(text) - 1;

        strncpy(text, open + 1, text_len);
        text[text_len] = '\0';

        cmd.type = CMD_PRINT;
        cmd.repeat = 1;

        char *star = strchr(close + 1, '*');

        if (star) {

            char *num = star + 1;

            while (*num == ' ')
                num++;

            if (*num != '\0')
                cmd.repeat = atoi(num);

            if (cmd.repeat <= 0)
                cmd.repeat = 1;
        }

        snprintf(cmd.argument,
                 sizeof(cmd.argument),
                 "%s",
                 text);

        return cmd;
    }

    /* ================================
       نفذ
    ================================= */

    if (strncmp(clean, كلمة_نفذ, strlen(كلمة_نفذ)) == 0)
    {
        const char *eq = strchr(clean, '=');

        if (!eq)
            return cmd;

        cmd.type = CMD_EXEC_SYSTEM;

        eq++;

        while (*eq == ' ')
            eq++;

        strncpy(cmd.argument, eq, sizeof(cmd.argument) - 1);
        cmd.argument[sizeof(cmd.argument) - 1] = '\0';

        return cmd;
    }


    if (strncmp(clean, "ادخل", 4) == 0)
    {


        char *eq = strchr(clean, '=');

        if (eq)
        {
            eq++; // بعد =

            while (*eq == ' ')
            eq++;

            strncpy(cmd.argument, eq, sizeof(cmd.argument) - 1);
            cmd.argument[sizeof(cmd.argument) - 1] = '\0';

            cmd.type = CMD_INPUT;
            return cmd;
        }
    }
    
    /* ================================
       تعيين متغير
    ================================= */

    if (strchr(clean, '='))
    {
        cmd.type = CMD_ASSIGN;

        strncpy(cmd.argument, clean, sizeof(cmd.argument) - 1);
        cmd.argument[sizeof(cmd.argument) - 1] = '\0';

        return cmd;
    }

    return cmd;
}
