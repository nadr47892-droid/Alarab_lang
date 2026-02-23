#include <stdio.h>
#include <string.h>
#include "محرك_الدالة.h"

#define MAX_FUNCTIONS 100
#define MAX_BODY 2048

typedef struct {
    char name[64];
    char body[MAX_BODY];
} Function;

static Function functions[MAX_FUNCTIONS];
static int function_count = 0;

void function_store(const char *name, const char *body) {
    if (function_count >= MAX_FUNCTIONS)
        return;

    strcpy(functions[function_count].name, name);
    strcpy(functions[function_count].body, body);
    function_count++;
}

const char *function_get(const char *name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return functions[i].body;
        }
    }
    return NULL;
}

