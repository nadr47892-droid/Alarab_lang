#include <string.h>
#include "محرك_المتغيرات.h"

typedef struct { char name[32]; double value; } Variable;
static Variable vars[100]; 
static int count = 0;

void var_set(const char *name, double value) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) { vars[i].value = value; return; }
    }
    strcpy(vars[count].name, name);
    vars[count++].value = value;
}

int var_get(const char *name, double *value) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) { *value = vars[i].value; return 1; }
    }
    return 0;
}
