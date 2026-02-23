#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "مفسر_العمليات.h"
#include "القاموس.h"


static void trim(char *str) {
    if (!str) return;

    int start = 0;
    while (isspace((unsigned char)str[start]))
        start++;

    if (start > 0)
        memmove(str, str + start, strlen(str + start) + 1);

    int end = strlen(str) - 1;
    while (end >= 0 && isspace((unsigned char)str[end])) {
        str[end] = '\0';
        end--;
    }
}

Command parse_line(const char *line) {

    Command cmd;
    cmd.type = CMD_UNKNOWN;
    cmd.name[0] = '\0';
    cmd.argument[0] = '\0';

    if (!line)
        return cmd;

    char clean[512];

    strncpy(clean, line, sizeof(clean) - 1);
    clean[sizeof(clean) - 1] = '\0';

    clean[strcspn(clean, "\n")] = '\0';
    trim(clean);

    if (strlen(clean) == 0)
        return cmd;

    /* ================================
       1. تعريف دالة
    ================================= */
    if (strncmp(clean, "دالة(", strlen("دالة(")) == 0) {

        const char *start = clean + strlen("دالة(");
        const char *end = strchr(start, ')');

        if (!end)
            return cmd;  // صيغة غير صحيحة → تجاهل

        int len = end - start;
        if (len <= 0)
            return cmd;

        if (len > 63)
            len = 63;

        cmd.type = CMD_FUNCTION_DEF;
        strncpy(cmd.name, start, len);
        cmd.name[len] = '\0';

        return cmd;
    }

    /* ================================
       2. استدعاء دالة
    ================================= */
    int len = strlen(clean);

    if (len > 2 && strcmp(clean + len - 2, "()") == 0) {

        int name_len = len - 2;
        if (name_len <= 0)
            return cmd;

        if (name_len > 63)
            name_len = 63;

        cmd.type = CMD_FUNCTION_CALL;
        strncpy(cmd.name, clean, name_len);
        cmd.name[name_len] = '\0';

        return cmd;
    }

    /* ================================
       3. كلمات ثابتة
    ================================= */
    if (strcmp(clean, كلمة_خروج) == 0) {
        cmd.type = CMD_EXIT;
        return cmd;
    }

    if (strcmp(clean, كلمة_نظف) == 0) {
        cmd.type = CMD_CLEAR;
        return cmd;
    }

    /* ================================
       4. أمر حل المعادلات (آمن بالكامل)
    ================================= */
    if (strncmp(clean, "حل", 2) == 0) {

        char *first_equal = strchr(clean, '=');

        if (!first_equal)
            return cmd;   // لا يوجد = → لا نحل

        char *eq_part = first_equal + 1;

        while (*eq_part == ' ')
            eq_part++;

        if (*eq_part == '\0')
            return cmd;   // لا يوجد شيء بعد =

        char *second_equal = strchr(eq_part, '=');

        cmd.type = CMD_SOLVE;

        if (second_equal) {

            *second_equal = '\0';

            if (*(second_equal + 1) == '\0')
                return cmd;

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
       5. اطبع
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

        /* استخراج النص داخل () */
        char text[256];
        size_t text_len = close - open - 1;

        if (text_len >= sizeof(text))
            text_len = sizeof(text) - 1;

        strncpy(text, open + 1, text_len);
        text[text_len] = '\0';

        /* إزالة علامات الاقتباس */
        size_t real_len = strlen(text);
        if (real_len >= 2 && text[0] == '"' && text[real_len - 1] == '"') {
            text[real_len - 1] = '\0';
            memmove(text, text + 1, real_len - 1);
        }

        cmd.type = CMD_PRINT;
        cmd.repeat = 1;

        /* فحص وجود * */
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
       6. احسب
    ================================= */
    if (strncmp(clean, كلمة_احسب, strlen(كلمة_احسب)) == 0) {

        char *eq = strchr(clean, '=');
        if (!eq)
            return cmd;

        if (*(eq + 1) == '\0')
            return cmd;

        cmd.type = CMD_EXEC;
        snprintf(cmd.argument,
                 sizeof(cmd.argument),
                 "%s",
                 eq + 1);

        return cmd;
    }

    if (strncmp(clean, كلمة_نفذ, strlen(كلمة_نفذ)) == 0) {

        const char *eq = strchr(clean, '=');
        if (!eq) return cmd;

        cmd.type = CMD_EXEC_SYSTEM;
        eq++;

        while (*eq == ' ') eq++;

        strncpy(cmd.argument, eq, sizeof(cmd.argument) - 1);
        cmd.argument[sizeof(cmd.argument) - 1] = '\0';

        return cmd;
    }


    /* ================================
       7. تعيين متغير
    ================================= */
    if (strchr(clean, '=')) {

        cmd.type = CMD_ASSIGN;

        size_t len = strlen(clean);

        if (len >= sizeof(cmd.argument))
            len = sizeof(cmd.argument) - 1;

        memcpy(cmd.argument, clean, len);
        cmd.argument[len] = '\0';

       return cmd;
    }

    return cmd;
}

