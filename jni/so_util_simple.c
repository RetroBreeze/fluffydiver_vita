/*
 * Simple so-loader implementation for Vita
 * Based on successful ports like Dead Space, Galaxy on Fire 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <kubridge.h>
#include "so_util_simple.h"

// Memory management
static SceUID so_memblock = -1;
static void *so_memory = NULL;
static size_t so_size = 0;

// Simple ELF header structures (minimal)
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

// Module state
typedef struct {
    void *base;
    size_t size;
    int loaded;
} simple_so_module;

static simple_so_module g_module = {0};

/*
 * Load .so file into memory
 */
int so_file_load(void *unused, const char *filename, uintptr_t load_addr) {
    printf("[SO] Loading %s at 0x%08x\n", filename, load_addr);
    
    // Open file
    SceUID fd = sceIoOpen(filename, SCE_O_RDONLY, 0);
    if (fd < 0) {
        printf("[SO] Failed to open file: %s (error: 0x%08x)\n", filename, fd);
        return -1;
    }
    
    // Get file size
    SceOff size = sceIoLseek(fd, 0, SCE_SEEK_END);
    if (size < 0) {
        printf("[SO] Failed to get file size\n");
        sceIoClose(fd);
        return -1;
    }
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    
    printf("[SO] File size: %d bytes\n", (int)size);
    
    // Allocate memory
    so_memblock = sceKernelAllocMemBlock("so_loader", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 
                                         (size + 0xFFF) & ~0xFFF, NULL);
    if (so_memblock < 0) {
        printf("[SO] Failed to allocate memory block\n");
        sceIoClose(fd);
        return -1;
    }
    
    sceKernelGetMemBlockBase(so_memblock, &so_memory);
    so_size = size;
    
    // Read file
    int ret = sceIoRead(fd, so_memory, size);
    if (ret != size) {
        printf("[SO] Failed to read file: %d != %d\n", ret, (int)size);
        sceIoClose(fd);
        return -1;
    }
    
    sceIoClose(fd);
    
    // Store module info
    g_module.base = so_memory;
    g_module.size = size;
    g_module.loaded = 1;
    
    printf("[SO] Loaded successfully at %p\n", so_memory);
    return 0;
}

/*
 * Basic relocation (minimal implementation)
 */
int so_relocate(void *unused) {
    printf("[SO] Relocating symbols...\n");
    
    if (!g_module.loaded) {
        printf("[SO] Module not loaded\n");
        return -1;
    }
    
    // For now, just return success
    // Real implementation would parse ELF and fix relocations
    printf("[SO] Relocation complete (stub)\n");
    return 0;
}

/*
 * Initialize module
 */
void so_initialize(void *unused) {
    printf("[SO] Initializing module...\n");
    
    if (!g_module.loaded) {
        printf("[SO] Module not loaded\n");
        return;
    }
    
    // Call constructors if present
    printf("[SO] Initialization complete\n");
}

/*
 * Find symbol by name
 */
uintptr_t so_symbol(void *unused, const char *symbol) {
    printf("[SO] Looking for symbol: %s\n", symbol);
    
    if (!g_module.loaded) {
        printf("[SO] Module not loaded\n");
        return 0;
    }
    
    // For now, return a dummy address
    // Real implementation would parse symbol table
    printf("[SO] Symbol lookup not implemented yet\n");
    return 0;
}

/*
 * Cleanup
 */
void so_cleanup() {
    if (so_memblock >= 0) {
        sceKernelFreeMemBlock(so_memblock);
        so_memblock = -1;
    }
    so_memory = NULL;
    so_size = 0;
    g_module.loaded = 0;
}

/*
 * Get module info
 */
int so_is_loaded() {
    return g_module.loaded;
}

void *so_get_base() {
    return g_module.base;
}

size_t so_get_size() {
    return g_module.size;
}
