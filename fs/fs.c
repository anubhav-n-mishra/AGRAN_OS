#include "fs.h"
#include "../include/kernel.h"
#include <stddef.h>

// Define the files array
struct File files[MAX_FILES];

// Initialize file system
void init_fs(void) {
    // Initialize all file slots as unused
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].is_used = 0;
        files[i].name[0] = '\0';
        files[i].content[0] = '\0';
    }
}

// Create a new file
void create_file(const char* name) {
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].is_used) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        print_string("Error: No free file slots\n");
        return;
    }
    
    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used && strcmp(files[i].name, name) == 0) {
            print_string("Error: File already exists\n");
            return;
        }
    }
    
    // Initialize new file
    strncpy(files[slot].name, name, MAX_FILENAME - 1);
    files[slot].name[MAX_FILENAME - 1] = '\0';
    files[slot].content[0] = '\0';
    files[slot].is_used = 1;
    
    print_string("Created file: ");
    print_string(name);
    print_char('\n');
}

// Delete a file
void delete_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used && strcmp(files[i].name, name) == 0) {
            files[i].is_used = 0;
            files[i].name[0] = '\0';
            files[i].content[0] = '\0';
            // No extra output to avoid prompt jump
            return;
        }
    }
    // No extra output to avoid prompt jump
}

// Write content to a file
int write_file(const char* name, const char* content) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used && strcmp(files[i].name, name) == 0) {
            strncpy(files[i].content, content, MAX_CONTENT - 1);
            files[i].content[MAX_CONTENT - 1] = '\0';
            return 0;
        }
    }
    print_string("Error: File not found\n");
    return -1;
}

// Read content from a file
int read_file(const char* name, char* buffer) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used && strcmp(files[i].name, name) == 0) {
            strncpy(buffer, files[i].content, MAX_CONTENT - 1);
            buffer[MAX_CONTENT - 1] = '\0';
            return 0;
        }
    }
    print_string("Error: File not found\n");
    return -1;
}

// List all files
void list_files(void) {
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used) {
            print_string(files[i].name);
            print_char('\n');
            found = 1;
        }
    }
    if (!found) {
        print_string("No files.\n");
    }
}

// Search for a file by name
int search_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].is_used && strcmp(files[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}