#include "../include/kernel.h"
#include "../fs/fs.h"
#include "../process/process.h"
#include "../kernel/screen.h"
#include "../kernel/keyboard.h"
#include "commands.h"
#include "math_commands.h"
#include "shell.h"
#include <stddef.h>

#define MAX_ARGS 16
#define MAX_CONTENT 1024

// RTC CMOS ports
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Helper to read a byte from CMOS
static uint8_t read_cmos(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// Convert BCD to binary
static int bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

// Helper function to parse command string into argc/argv
static int parse_command(const char* command, char* argv[]) {
    int argc = 0;
    static char cmd_copy[MAX_CONTENT];
    
    // Make a copy of the command
    int i;
    for(i = 0; command[i] != '\0' && i < MAX_CONTENT - 1; i++) {
        cmd_copy[i] = command[i];
    }
    cmd_copy[i] = '\0';
    
    char* token = cmd_copy;
    
    // Skip leading spaces
    while (*token == ' ') token++;
    
    // Parse arguments
    while (*token && argc < MAX_ARGS) {
        argv[argc++] = token;
        
        // Find end of argument
        while (*token && *token != ' ') token++;
        
        // Replace space with null terminator
        if (*token) *token++ = '\0';
        
        // Skip spaces between arguments
        while (*token == ' ') token++;
    }
    
    return argc;
}

// Built-in commands
void cmd_help(int argc, char* argv[]) {
    if (argc == 1) {
        print_string("\n=== Help Categories ===\n\n");
        print_string("filesystem  - File management commands\n");
        print_string("process     - Process management commands\n");
        print_string("system      - System commands\n");
        print_string("math        - Calculator and math commands\n");
        print_string("date/time   - Date, time, and clock commands\n");
        print_string("history     - Show command history\n");
        print_string("Type 'help <category>' to see commands in that category.\n");
        return;
    }
    if (strcmp(argv[1], "filesystem") == 0) {
        print_string("\nFile System Commands:\n");
        print_string("ls        - List all files in system\n");
        print_string("create    - Create a new file (create filename)\n");
        print_string("write     - Write text to file (write filename text)\n");
        print_string("read      - Read file contents (read filename)\n");
        print_string("delete    - Delete a file (delete filename)\n");
        print_string("search    - Search for a file by name (search filename)\n");
    } else if (strcmp(argv[1], "process") == 0) {
        print_string("\nProcess Management Commands:\n");
        print_string("ps        - Show all running processes\n");
        print_string("run       - Start a new process (run processname)\n");
        print_string("kill      - Stop a process (kill pid)\n");
        print_string("demo      - Run process scheduling demo\n");
    } else if (strcmp(argv[1], "system") == 0) {
        print_string("\nSystem Commands:\n");
        print_string("clear     - Clear screen\n");
        print_string("echo      - Echo the arguments\n");
        print_string("info      - Show system information\n");
        print_string("version   - Show OS version\n");
        print_string("shutdown  - Shutdown the system\n");
        print_string("reboot    - Reboot the system\n");
        print_string("font      - Change text color (font red/green/yellow/blue/magenta/cyan/white)\n");
        print_string("            Supported colors: red, green, yellow, blue, magenta, cyan, white\n");
    } else if (strcmp(argv[1], "math") == 0) {
        print_string("\nMath/Calculator Commands:\n");
        print_string("calculator - Enter calculator mode (type expressions like 2+3, type 'exit' to quit)\n");
    } else if (strcmp(argv[1], "date") == 0 || strcmp(argv[1], "time") == 0) {
        print_string("\nDate/Time Commands:\n");
        print_string("date       - Show current date\n");
        print_string("time       - Show current time\n");
    } else {
        print_string("Unknown help category. Type 'help' to see available categories.\n");
    }
}

void cmd_clear(void) {
    clear_screen();
}

void cmd_echo(const char* text) {
    if (text == NULL) {
        print_string("Usage: echo [text]\n");
        return;
    }
    print_string(text);
    print_string("\n");
}

void cmd_info(void) {
    print_string("\nAGRAN OS System Information\n");
    print_string("==========================\n");
    print_string("OS Name: AGRAN OS\n");
    print_string("Version: 1.0\n");
    print_string("Architecture: x86\n");
    print_string("Memory: 1.44 MB\n");
    print_string("Features:\n");
    print_string("- Basic File System\n");
    print_string("- Process Management\n");
    print_string("- Round Robin Scheduling\n");
    print_string("==========================\n");
}

void cmd_version(void) {
    print_string("AGRAN OS version 1.0\n");
}

void cmd_shutdown(void) {
    print_string("\nShutting down AGRAN OS...\n");
    print_string("It is now safe to turn off your computer.\n");
    shutdown();
}

void cmd_reboot(void) {
    print_string("\nRebooting AGRAN OS...\n");
    reboot();
}

// File system commands
void cmd_ls(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    list_files();
}

void cmd_create(int argc, char* argv[]) {
    if (argc < 2) {
        print_string("Usage: create <filename>\n");
        return;
    }
    create_file(argv[1]);
    // No extra debug or confirmation output to minimize scrolling
}

void cmd_write(int argc, char* argv[]) {
    if (argc < 3) {
        print_string("Usage: write <filename> <content>\n");
        return;
    }
    write_file(argv[1], argv[2]);
}

void cmd_read(int argc, char* argv[]) {
    if (argc < 2) {
        print_string("Usage: read <filename>\n");
        return;
    }
    char buffer[MAX_CONTENT];
    if (read_file(argv[1], buffer) >= 0) {
        print_string(buffer);
        print_string("\n");
    }
}

void cmd_delete(int argc, char* argv[]) {
    if (argc < 2) {
        print_string("Usage: delete <filename>\n");
        return;
    }
    delete_file(argv[1]);
}

// Process commands
void cmd_ps(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    display_processes();
}

void cmd_run(int argc, char* const argv[]) {
    if (argc < 2) {
        print_string("Error: Please provide a process name\n");
        print_string("Usage: run <process_name>\n");
        return;
    }

    // Create the process with a default burst time
    int pid = create_process(argv[1], DEFAULT_QUANTUM);
    if (pid >= 0) {
        print_string("Created process '");
        print_string(argv[1]);
        print_string("' with PID ");
        print_int(pid);
        print_string("\n");
    } else {
        print_string("Error: Failed to create process '");
        print_string(argv[1]);
        print_string("'\n");
    }
}

void cmd_kill(int argc, char* const argv[]) {
    if (argc < 2) {
        print_string("Usage: kill <pid>\n");
        return;
    }
    int pid = string_to_int(argv[1]);
    if (is_process_alive(pid)) {
        kill_process(pid);
        print_string("Killed process with PID ");
        print_string(argv[1]);
        print_string("\n");
    } else {
        print_string("No such process with PID ");
        print_string(argv[1]);
        print_string("\n");
    }
}

// Demo commands
void cmd_demo(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    print_string("\n=== Process Scheduling Demo ===\n");
    
    // Create test processes with shorter quantum times
    create_process("Task1", 2);
    create_process("Task2", 2);
    create_process("Task3", 2);
    
    // Run scheduling for a limited number of cycles
    print_string("\nRunning processes for demo...\n");
    for (int i = 0; i < 6; i++) {
        schedule();
        // Add a small delay between schedules
        for (volatile int j = 0; j < 1000000; j++) {}
    }
    
    // Clean up any remaining processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (is_process_alive(i)) {
            kill_process(i);
        }
    }
    
    print_string("\nDemo completed.\n");
}

void cmd_filedemo(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    print_string("\n=== File System Demo ===\n");
    create_file("test.txt");
    write_file("test.txt", "Hello, AGRAN OS!");
    print_string("Reading test.txt: ");
    char buffer[256];
    read_file("test.txt", buffer);
    print_string(buffer);
    print_char('\n');
    list_files();
    delete_file("test.txt");
    list_files();
}

// Math/Calculator command
void cmd_calculator(int argc, char* argv[]) {
    // Join all arguments except the command itself into a single string
    char args_str[128] = {0};
    int offset = 0;
    for (int i = 1; i < argc; i++) {
        int n = 0;
        if (i > 1) {
            args_str[offset++] = ' ';
        }
        n = strlen(argv[i]);
        strncpy(&args_str[offset], argv[i], sizeof(args_str) - offset - 1);
        offset += n;
        if (offset >= (int)sizeof(args_str) - 1) break;
    }
    calculator_command(args_str);
}

void cmd_date(void) {
    int day = bcd_to_bin(read_cmos(0x07));
    int month = bcd_to_bin(read_cmos(0x08));
    int year = bcd_to_bin(read_cmos(0x09));
    print_string("Date: ");
    print_int(day); print_string("/");
    print_int(month); print_string("/");
    print_int(2000 + year); print_string("\n");
}

void cmd_time(void) {
    int hour = bcd_to_bin(read_cmos(0x04));
    int min  = bcd_to_bin(read_cmos(0x02));
    int sec  = bcd_to_bin(read_cmos(0x00));
    print_string("Time: ");
    print_int(hour); print_string(":");
    if (min < 10) print_string("0");
    print_int(min); print_string(":");
    if (sec < 10) print_string("0");
    print_int(sec); print_string("\n");
}

// Command execution
void execute_command(const char* command) {
    char* argv[MAX_ARGS];
    int argc = parse_command(command, argv);
    
    if (argc == 0) return;
    
    // System commands
    if (strcmp(argv[0], "help") == 0) cmd_help(argc, argv);
    else if (strcmp(argv[0], "clear") == 0) cmd_clear();
    else if (strcmp(argv[0], "echo") == 0) cmd_echo(argc > 1 ? argv[1] : NULL);
    else if (strcmp(argv[0], "info") == 0) cmd_info();
    else if (strcmp(argv[0], "version") == 0) cmd_version();
    else if (strcmp(argv[0], "shutdown") == 0) cmd_shutdown();
    else if (strcmp(argv[0], "reboot") == 0) cmd_reboot();
    
    // File system commands
    else if (strcmp(argv[0], "ls") == 0) cmd_ls(argc, argv);
    else if (strcmp(argv[0], "create") == 0) cmd_create(argc, argv);
    else if (strcmp(argv[0], "write") == 0) cmd_write(argc, argv);
    else if (strcmp(argv[0], "read") == 0) cmd_read(argc, argv);
    else if (strcmp(argv[0], "delete") == 0) cmd_delete(argc, argv);
    else if (strcmp(argv[0], "search") == 0) cmd_search(argc, argv);
    
    // Process commands
    else if (strcmp(argv[0], "ps") == 0) cmd_ps(argc, argv);
    else if (strcmp(argv[0], "run") == 0) cmd_run(argc, argv);
    else if (strcmp(argv[0], "kill") == 0) cmd_kill(argc, argv);
    else if (strcmp(argv[0], "demo") == 0) cmd_demo(argc, argv);
    else if (strcmp(argv[0], "calculator") == 0) cmd_calculator(argc, argv);
    else if (strcmp(argv[0], "date") == 0) cmd_date();
    else if (strcmp(argv[0], "time") == 0) cmd_time();
    else if (strcmp(argv[0], "history") == 0) cmd_history();
    else if (strcmp(argv[0], "font") == 0) cmd_font(argc, argv);
    
    else {
        print_string("Unknown command: ");
        print_string(argv[0]);
        print_string("\nType 'help' for available commands\n");
    }
}

void cmd_history(void) {
    int count = get_history_count();
    print_string("Command History:\n");
    for (int i = 0; i < count && i < HISTORY_SIZE; i++) {
        print_int(i + 1);
        print_string(": ");
        print_string(get_history_entry(i));
        print_string("\n");
    }
}

void cmd_search(int argc, char* argv[]) {
    if (argc < 2) {
        print_string("Usage: search <filename>\n");
        return;
    }
    if (search_file(argv[1])) {
        print_string("Found: ");
        print_string(argv[1]);
        print_string("\n");
    } else {
        print_string("File not found\n");
    }
}

void cmd_font(int argc, char* argv[]) {
    if (argc < 2) {
        print_string("Usage: font <color>\n");
        return;
    }
    uint8_t color = 0x07; // Default white on black
    if (strcmp(argv[1], "red") == 0) color = 0x04;
    else if (strcmp(argv[1], "green") == 0) color = 0x02;
    else if (strcmp(argv[1], "yellow") == 0) color = 0x0E;
    else if (strcmp(argv[1], "blue") == 0) color = 0x01;
    else if (strcmp(argv[1], "magenta") == 0) color = 0x05;
    else if (strcmp(argv[1], "cyan") == 0) color = 0x03;
    else if (strcmp(argv[1], "white") == 0) color = 0x07;
    else if (strcmp(argv[1], "black") == 0) {
        print_string("Black color is not supported. Default is white on black.\n");
        return;
    }
    else {
        print_string("Unknown color. Supported: red, green, yellow, blue, magenta, cyan, white\n");
        return;
    }
    set_text_color(color);
    print_string("Text color changed.\n");
}