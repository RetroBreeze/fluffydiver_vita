#ifndef SO_UTIL_SIMPLE_H
#define SO_UTIL_SIMPLE_H

#include <stdint.h>
#include <stddef.h>

/*
 * Simple so-loader API
 * Compatible with the main so-loader interface
 */

// Dummy module structure for compatibility
typedef struct {
    int dummy;
} so_module;

// Function declarations
int so_file_load(void *unused, const char *filename, uintptr_t load_addr);
int so_relocate(void *unused);
void so_initialize(void *unused);
uintptr_t so_symbol(void *unused, const char *symbol);

// Utility functions
void so_cleanup();
int so_is_loaded();
void *so_get_base();
size_t so_get_size();

#endif // SO_UTIL_SIMPLE_H
