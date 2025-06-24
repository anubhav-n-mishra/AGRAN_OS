#include "kernel.h"
#include "screen.h"
#include "keyboard.h"
#include "../process/process.h"
#include "../shell/shell.h"
#include "../fs/fs.h"
#include <stddef.h>

// Video memory constants
#define VIDEO_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_WHITE_ON_BLACK 0x07
#define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)

// Global variables
static uint16_t* const video_memory = (uint16_t*)VIDEO_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static int shift_pressed = 0;  // Track shift key state
static uint8_t current_text_color = VGA_WHITE_ON_BLACK; // 0x07, white on black

// Function declarations (only for static functions)
static void display_boot_logo(void);

// Keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// QEMU/Bochs shutdown ports
#define QEMU_SHUTDOWN_PORT 0x604
#define BOCHS_SHUTDOWN_PORT 0x8900

// Keyboard scancodes
#define SCANCODE_PAGE_UP 0x49
#define SCANCODE_PAGE_DOWN 0x51
#define SCANCODE_SHIFT 0x2A
#define SCANCODE_SHIFT_RELEASE 0xAA
#define SCANCODE_EXTENDED 0xE0
#define SCANCODE_UP_ARROW 0x48
#define SCANCODE_DOWN_ARROW 0x50

// Boot logo
static const char* BOOT_LOGO[] = {
    "",
    "",
    "W e l c o m e   t o   A G R A N   O S",
    "",
    "L o a d i n g . . .",
    "",
    NULL
};

// Keyboard scancode to ASCII mapping
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Numpad scancode to ASCII mapping (partial, for common keys)
static const struct { uint8_t scancode; char ascii; } numpad_map[] = {
    {0x52, '0'}, {0x4F, '1'}, {0x50, '2'}, {0x51, '3'},
    {0x4B, '4'}, {0x4C, '5'}, {0x4D, '6'},
    {0x47, '7'}, {0x48, '8'}, {0x49, '9'},
    {0x4A, '-'}, {0x4E, '+'}, {0x37, '*'}, {0x35, '/'},
};
#define NUMPAD_MAP_SIZE (sizeof(numpad_map)/sizeof(numpad_map[0]))

// Update hardware cursor position
void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, pos >> 8);            // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, pos & 0xFF);          // Send the low cursor byte.
}

// Scroll the screen up by one line (classic terminal behavior)
static void scroll_screen() {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            video_memory[(y - 1) * VGA_WIDTH + x] = video_memory[y * VGA_WIDTH + x];
        }
    }
    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        video_memory[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)VGA_WHITE_ON_BLACK << 8;
    }
}

// Print string to screen
void print_string(const char* str) {
    for(int i = 0; str[i] != '\0'; i++) {
        if(str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
            if(cursor_y >= VGA_HEIGHT) {
                scroll_screen();
                cursor_y = VGA_HEIGHT - 1;
            }
            update_cursor();
            continue;
        }
        if(str[i] == '\b') {
            backspace();
            continue;
        }
        if(cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            if(cursor_y >= VGA_HEIGHT) {
                scroll_screen();
                cursor_y = VGA_HEIGHT - 1;
            }
            update_cursor();
        }
        video_memory[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)str[i] | (uint16_t)current_text_color << 8;
        cursor_x++;
        update_cursor();
    }
}

char getchar(void) {
    static int extended = 0;
    while(1) {
        if((inb(KEYBOARD_STATUS_PORT) & 1) != 0) {
            uint8_t scancode = inb(KEYBOARD_DATA_PORT);
            if(scancode == SCANCODE_EXTENDED) {
                extended = 1;
                continue;
            }
            if(extended) {
                extended = 0;
                if(scancode == SCANCODE_UP_ARROW) return 0x80;   // Up arrow
                if(scancode == SCANCODE_DOWN_ARROW) return 0x81; // Down arrow
                continue;
            }
            // Ignore key releases (when the highest bit is set)
            if(scancode & 0x80) {
                continue;
            }
            // Handle special keys
            switch(scancode) {
                case SCANCODE_SHIFT:
                    shift_pressed = 1;
                    continue;
                case SCANCODE_SHIFT_RELEASE:
                    shift_pressed = 0;
                    continue;
                case SCANCODE_PAGE_UP:
                case SCANCODE_UP_ARROW:
                    if(shift_pressed) {
                        //scroll_up();
                        continue;
                    }
                    break;
                case SCANCODE_PAGE_DOWN:
                case SCANCODE_DOWN_ARROW:
                    if(shift_pressed) {
                        //scroll_down();
                        continue;
                    }
                    break;
            }
            // Numpad support
            for (unsigned i = 0; i < NUMPAD_MAP_SIZE; ++i) {
                if (scancode == numpad_map[i].scancode) {
                    return numpad_map[i].ascii;
                }
            }
            // Convert scancode to ASCII if in valid range
            if(scancode < sizeof(scancode_to_ascii)) {
                char c = scancode_to_ascii[scancode];
                if(c != 0) {
                    if(shift_pressed && c >= 'a' && c <= 'z') {
                        c = c - 'a' + 'A';
                    }
                    return c;
                }
            }
        }
    }
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static void display_boot_logo(void) {
    clear_screen();
    
    // Calculate vertical position to center the logo
    int logo_height = 0;
    for(int i = 0; BOOT_LOGO[i] != NULL; i++) {
        logo_height++;
    }
    
    int start_y = (VGA_HEIGHT - logo_height) / 2;
    
    // Display each line of the logo with animation
    for(int i = 0; BOOT_LOGO[i] != NULL; i++) {
        // Calculate horizontal position to center the line
        const char* line = BOOT_LOGO[i];
        int line_length = 0;
        while(line[line_length] != '\0') line_length++;
        
        cursor_x = (VGA_WIDTH - line_length) / 2;
        cursor_y = start_y + i;
        
        // Print each character with a small delay
        for(int j = 0; line[j] != '\0'; j++) {
            char str[2] = {line[j], '\0'};
            print_string(str);
            // Increased character delay
            for(volatile int k = 0; k < 5000000; k++) {}
        }
        
        // Increased delay between lines
        for(volatile int j = 0; j < 20000000; j++) {}
    }
    
    // Add loading dots animation
    cursor_y = start_y + 5;  // Position at "Loading" line
    cursor_x = (VGA_WIDTH + 11) / 2;  // Position after "Loading"
    
    // Animate three dots with increased delay
    for(int dots = 0; dots < 3; dots++) {
        for(volatile int i = 0; i < 50000000; i++) {}  // Increased delay before each dot
        print_string(".");
    }
    
    // Final delay showing complete logo
    for(volatile int i = 0; i < 80000000; i++) {}  // Increased final delay
}

// Kernel entry point
void __attribute__((section(".text.boot"))) kmain(void) {
    // Initialize hardware
    init_screen();
    init_keyboard();
    
    // Initialize subsystems
    init_scheduler();  // Initialize process scheduler
    init_fs();        // Initialize file system
    
    // Display boot logo
    display_boot_logo();

    // Clear screen after boot logo so shell starts at top
    clear_screen();

    // Start the shell
    init_shell();
    run_shell();
    
    // Should never reach here
    while(1) {
        asm volatile("hlt");
    }
}

void shutdown(void) {
    asm volatile("cli"); // Disable interrupts
    outw(QEMU_SHUTDOWN_PORT, 0x2000);
    outw(BOCHS_SHUTDOWN_PORT, 0x8900);
    print_string("Failed to shutdown. System halted.\n");
    while(1) { asm volatile("cli; hlt"); }
}

void reboot(void) {
    asm volatile("cli");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    while(1) { asm volatile("cli; hlt"); }
}

// Video functions
void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            clear_screen();
            cursor_y = VGA_HEIGHT - 1;
        }
        update_cursor();
        return;
    }
    
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            const int index = cursor_y * VGA_WIDTH + cursor_x;
            video_memory[index] = (uint16_t)' ' | (uint16_t)current_text_color << 8;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
            const int index = cursor_y * VGA_WIDTH + cursor_x;
            video_memory[index] = (uint16_t)' ' | (uint16_t)current_text_color << 8;
        }
        update_cursor();
        return;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            clear_screen();
            cursor_y = VGA_HEIGHT - 1;
        }
    }
    
    const int index = cursor_y * VGA_WIDTH + cursor_x;
    video_memory[index] = (uint16_t)c | (uint16_t)current_text_color << 8;
    cursor_x++;
    update_cursor();
}

void print_int(int num) {
    char str[32];
    int i = 0;
    
    // Handle negative numbers
    if (num < 0) {
        print_char('-');
        num = -num;
    }
    
    // Convert to string
    do {
        str[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
    
    // Print in reverse order
    while (i > 0) {
        print_char(str[--i]);
    }
}

void clear_screen(void) {
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = (uint16_t)' ' | (uint16_t)VGA_WHITE_ON_BLACK << 8;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void init_video(void) {
    clear_screen();
    update_cursor();
}

// String conversion functions
void int_to_string(int num, char* str) {
    int i = 0;
    int is_negative = 0;

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Handle case of 0
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Process individual digits
    while (num != 0) {
        str[i++] = '0' + (num % 10);
        num = num / 10;
    }

    // Add negative sign if needed
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int string_to_int(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Handle negative numbers
    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    // Process each digit
    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        }
        i++;
    }

    return sign * result;
}

// Initialize screen
void init_screen(void) {
    clear_screen();
    cursor_x = 0;
    cursor_y = 0;
    current_text_color = VGA_WHITE_ON_BLACK; // Always reset to white on black at startup
    update_cursor();
}

// Initialize keyboard
void init_keyboard(void) {
    // Nothing needed for basic keyboard initialization
    // The keyboard is already initialized by the BIOS
    shift_pressed = 0;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void set_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

int get_cursor_y(void) { return cursor_y; }

void set_text_color(uint8_t color) {
    current_text_color = color;
}

uint8_t get_text_color(void) {
    return current_text_color;
}