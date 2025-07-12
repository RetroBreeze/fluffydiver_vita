/*
 * so_util.h - Header for so-loader utilities
 */

#ifndef SO_UTIL_H
#define SO_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <psp2/kernel/sysmem.h>

// Module structure
typedef struct {
    void *base;                  // Base address of loaded module
    size_t size;                 // Size of loaded module
    SceUID memblock;             // Memory block ID
    void *symbols;               // Symbol table (opaque pointer)
    int symbol_count;            // Number of symbols
    char *strings;               // String table
} so_module;

// Function declarations
int so_file_load(so_module *mod, const char *filename, uintptr_t load_addr);
int so_relocate(so_module *mod);
uintptr_t so_symbol(so_module *mod, const char *symbol_name);
void hook_addr(uintptr_t addr, uintptr_t dst);
void so_cleanup(so_module *mod);

#endif // SO_UTIL_H
