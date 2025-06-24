#ifndef MATH_COMMANDS_H
#define MATH_COMMANDS_H

// Calculator command handler
typedef enum {
    CALC_ADD,
    CALC_SUB,
    CALC_MUL,
    CALC_DIV
} calc_op_t;

void calculator_command(const char* args);

#endif // MATH_COMMANDS_H
