#ifndef SO_UTIL_SIMPLE_H
#define SO_UTIL_SIMPLE_H

#include <stdint.h>
#include <stddef.h>

/*
 * Enhanced so-loader API with actual function dispatch
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

// NEW: Function dispatcher for calling actual game code
void so_call_game_init();
void so_call_game_update();
void so_call_game_touch(int x, int y, int action);
void so_call_game_pause();
void so_call_game_resume();
void so_call_game_back();

// NEW: Check if functions are resolved
int so_functions_resolved();

#endif // SO_UTIL_SIMPLE_H
