#ifndef __SO_UTIL_H__
#define __SO_UTIL_H__

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char *symbol;
    uintptr_t func;
} DynLibFunction;

typedef struct so_module {
    void *base;
    size_t size;
    void *dynsym;
    void *dynstr;
    void *hash;
    size_t dynsym_num;
    void *text_base;
    size_t text_size;
    void *rel;           // DT_REL
    size_t rel_size;     // DT_RELSZ
    void *plt_rel;       // DT_JMPREL
    size_t plt_rel_size; // DT_PLTRELSZ
} so_module;

// Core so-loader functions
int so_file_load(so_module *mod, const char *filename, uintptr_t load_addr);
int so_load(so_module *mod, const char *path, uintptr_t load_addr);
int so_relocate(so_module *mod);
int so_resolve(so_module *mod, DynLibFunction *funcs, size_t num_funcs, int strict);
void so_flush_caches(so_module *mod);
int so_initialize(so_module *mod);
uintptr_t so_symbol(so_module *mod, const char *symbol);
void hook_addr(uintptr_t addr, uintptr_t dst);

// CRITICAL FIX: Add missing function declaration
int so_analyze_and_try_symbols(so_module *mod, void *fake_env, void *fake_context);

// Utility functions
int ret0();
int ret1();
int retminus1();
void *retNULL();

// Global variables
extern void *fake_env;
extern void *fake_context;
extern DynLibFunction default_dynlib[];
extern size_t default_dynlib_size;

#endif
