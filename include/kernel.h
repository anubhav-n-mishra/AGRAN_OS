#ifndef KERNEL_H
#define KERNEL_H

#define NULL ((void*)0)

#include <stddef.h>
#include <stdint.h>

// I/O port functions for x86
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Video functions
void init_video(void);
void clear_screen(void);
void print_char(char c);
void print_string(const char* str);
void print_int(int num);
void update_cursor(void);
void scroll_up(void);
void scroll_down(void);
void set_cursor(int x, int y);
int get_cursor_y(void); // Added function declaration
void set_text_color(uint8_t color);
uint8_t get_text_color(void);

// Keyboard functions
char getchar(void);

// String functions
int strcmp(const char* s1, const char* s2);
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);

// System control functions
void shutdown(void);
void reboot(void);

// Shell functions
void init_shell(void);
void run_shell(void);

// Kernel functions
void kmain(void) __attribute__((section(".text.boot")));

// Utility functions
void int_to_string(int num, char* str);
int string_to_int(const char* str);

#endif