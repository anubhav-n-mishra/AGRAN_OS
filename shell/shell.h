#ifndef SHELL_H
#define SHELL_H

#include <string.h>

// Maximum command length and arguments
#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16

// History size
#define HISTORY_SIZE 64

// Shell functions
void init_shell(void);
void handle_input(char c);
void backspace(void);

// Access to shell history
int get_history_count(void);
const char* get_history_entry(int i);

#endif