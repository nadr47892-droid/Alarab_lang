#include "المحرك.h"
#include "مفسر_العمليات.h"
#include "محرك_الاشارات.h"

int engine_run_line(const char *line) {
    Command cmd = parse_line(line);
    return runtime_execute(cmd);   // ← يرجّع 1 عند خروج
}

