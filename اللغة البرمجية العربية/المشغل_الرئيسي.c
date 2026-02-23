#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "مفسر_العمليات.h"
#include "المنفذ.h"
#include "محرك_الدالة.h"

int main(int argc, char *argv[]) {

    char line[512];

    // =====================================
    // تشغيل ملف إذا تم تمرير اسم ملف
    // =====================================
    if (argc > 1) {

        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("خطأ: لا يمكن فتح الملف %s\n", argv[1]);
            return 1;
        }

        while (fgets(line, sizeof(line), file)) {

            line[strcspn(line, "\n")] = '\0';

            Command cmd = parse_line(line);

            // تجاهل السطور الفارغة
            if (cmd.type == CMD_UNKNOWN)
                continue;

            // ===============================
            // تعريف دالة داخل ملف
            // ===============================
            if (cmd.type == CMD_FUNCTION_DEF) {

                char body[2048] = "";
                char inner[512];

                while (fgets(inner, sizeof(inner), file)) {

                    inner[strcspn(inner, "\n")] = '\0';

                    if (strchr(inner, '}')) {
                        inner[strcspn(inner, "}")] = '\0';

                        if (strlen(inner) > 0) {
                            strcat(body, inner);
                            strcat(body, "\n");
                        }
                        break;
                    }

                    strcat(body, inner);
                    strcat(body, "\n");
                }

                function_store(cmd.name, body);
                continue;
            }

            if (execute_command(cmd))
                break;
        }

        fclose(file);
        return 0;
    }

    // =====================================
    // الوضع التفاعلي REPL
    // =====================================
    while (1) {

        printf("لغة> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        Command cmd = parse_line(line);

        // تجاهل الأسطر الفارغة
        if (cmd.type == CMD_UNKNOWN)
            continue;

        // ===============================
        // تعريف دالة
        // ===============================
        if (cmd.type == CMD_FUNCTION_DEF) {

            char body[2048] = "";
            char inner[512];

            while (1) {

                if (!fgets(inner, sizeof(inner), stdin))
                    break;

                inner[strcspn(inner, "\n")] = '\0';

                if (strchr(inner, '}')) {
                    inner[strcspn(inner, "}")] = '\0';

                    if (strlen(inner) > 0) {
                        strcat(body, inner);
                        strcat(body, "\n");
                    }
                    break;
                }

                strcat(body, inner);
                strcat(body, "\n");
            }

            function_store(cmd.name, body);
            continue;
        }

        if (execute_command(cmd))
            break;
    }

    return 0;
}

