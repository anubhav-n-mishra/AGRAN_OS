#include "math_commands.h"
#include "../include/kernel.h"

// Dummy implementations of print_string and print_int for illustration.
// These should be replaced with the actual implementations.
void print_string(const char* str);
void print_int(int num);

// Simple calculator command: usage example: calculator add 2 3
void calculator_command(const char* args) {
    char input[64];
    if (args && args[0]) {
        // If arguments are provided, treat as one-shot calculation
        strncpy(input, args, sizeof(input)-1);
        input[sizeof(input)-1] = '\0';
        goto single_eval;
    }
    print_string("Calculator mode. Type expressions like 2+3 or 'exit' to quit.\n");
    while (1) {
        print_string("> ");
        int pos = 0;
        char c = 0;
        // Read input from getchar()
        while ((c = getchar()) != '\n' && pos < (int)sizeof(input)-1) {
            if (c == '\b' && pos > 0) {
                pos--;
                print_string("\b \b");
            } else if (c != '\b') {
                input[pos++] = c;
                print_char(c);
            }
        }
        input[pos] = '\0';
        print_string("\n");
        if (strcmp(input, "exit") == 0) break;
    single_eval:
        // Manual parsing for a op b (e.g., 2+3, 10 - 4)
        int a = 0, b = 0, res = 0;
        char op = 0;
        int i = 0, sign = 1;
        // Skip leading spaces
        while (input[i] == ' ') i++;
        if (input[i] == '-') { sign = -1; i++; }
        while (input[i] >= '0' && input[i] <= '9') {
            a = a * 10 + (input[i++] - '0');
        }
        a *= sign;
        // Skip spaces
        while (input[i] == ' ') i++;
        op = input[i++];
        // Skip spaces
        while (input[i] == ' ') i++;
        sign = 1;
        if (input[i] == '-') { sign = -1; i++; }
        while (input[i] >= '0' && input[i] <= '9') {
            b = b * 10 + (input[i++] - '0');
        }
        b *= sign;
        if (op == '+') res = a + b;
        else if (op == '-') res = a - b;
        else if (op == '*') res = a * b;
        else if (op == '/') {
            if (b == 0) { print_string("Error: Division by zero!\n"); goto next; }
            res = a / b;
        } else { print_string("Invalid expression. Use format: 2+3 or 10 - 4\n"); goto next; }
        print_int(res); print_string("\n");
    next:
        if (args && args[0]) break;
    }
    print_string("Exiting calculator.\n");
}
