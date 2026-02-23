#ifndef PARSER_H
#define PARSER_H
#define MAX_NAME 64
#define MAX_ARG  256


typedef enum {
    CMD_PRINT,
    CMD_EXEC,
    CMD_EXIT,
    CMD_CLEAR,
    CMD_UNKNOWN,
    CMD_EXEC_SYSTEM,
    CMD_ASSIGN,
    CMD_FUNCTION_DEF,
    CMD_FUNCTION_CALL,
    CMD_SOLVE,
} CommandType;

typedef struct {
    CommandType type;
    char name[MAX_NAME];       // للدوال
    char argument[MAX_ARG];  // لباقي الأوامر
    int repeat;
} Command;

Command parse_line(const char *line);

#endif

