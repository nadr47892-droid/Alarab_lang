#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "المنفذ.h"
#include "محرك_الاشارات.h"
#include "المحرك_الرياضي.h"
#include "محرك_الدالة.h"
#include "مفسر_العمليات.h"

void execute_system_command(const char *cmd) {
    if (!cmd || cmd[0] == '\0') {
        printf("خطأ: لا يوجد أمر للتنفيذ\n");
        return;
    }
    system(cmd);
}

int execute_command(Command cmd) {

    // ===============================
    // استدعاء دالة
    // ===============================
    if (cmd.type == CMD_FUNCTION_CALL) {

        const char *body = function_get(cmd.name);

        if (body) {

            char temp[2048];
            strcpy(temp, body);

            char *line = strtok(temp, "\n");

            while (line != NULL) {

                Command inner_cmd = parse_line(line);

                // مهم: نستخدم execute_command لدعم الاستدعاء المتداخل
                if (execute_command(inner_cmd))
                    return 1;

                line = strtok(NULL, "\n");
            }

        } else {
            printf("خطأ: الدالة غير موجودة\n");
        }

        return 0;
    }

    // ===============================
    // تنفيذ أمر نظام
    // ===============================
    if (cmd.type == CMD_EXEC_SYSTEM) {
        execute_system_command(cmd.argument);
        return 0;
    }

    // ===============================
    // باقي الأوامر
    // ===============================
    if (runtime_execute(cmd))
        return 1;

    return 0;
}

