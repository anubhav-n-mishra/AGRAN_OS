#ifndef COMMANDS_H
#define COMMANDS_H

// File system commands
void cmd_ls(int argc, char* argv[]);
void cmd_create(int argc, char* argv[]);
void cmd_write(int argc, char* argv[]);
void cmd_read(int argc, char* argv[]);
void cmd_delete(int argc, char* argv[]);

// System commands
void cmd_help(int argc, char* argv[]);
void cmd_clear(void);
void cmd_echo(const char* text);
void cmd_info(void);
void cmd_version(void);
void cmd_shutdown(void);
void cmd_reboot(void);

// Process commands
void cmd_ps(int argc, char* argv[]);
void cmd_run(int argc, char* const argv[]);
void cmd_kill(int argc, char* const argv[]);

// Demo commands
void cmd_demo(int argc, char* argv[]);
void cmd_filedemo(int argc, char* argv[]);

// Math/Calculator command
void cmd_calculator(int argc, char* argv[]);

// Date and time commands
void cmd_date(void);
void cmd_time(void);

// Command execution
void execute_command(const char* command);

// History command
void cmd_history(void);

// Search command
void cmd_search(int argc, char* argv[]);

// Font command
void cmd_font(int argc, char* argv[]);

#endif