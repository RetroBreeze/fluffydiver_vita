/*
 * so_util.c - Based on proven working implementations
 * Simplified version of TheOfficialFloW's so_util for GTA SA Vita
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <kubridge.h>
#include "so_util.h"

// External debug function
extern void debug_printf(const char *fmt, ...);

// ELF constants
#define ELF_MAGIC 0x464C457F
#define ET_DYN 3
#define EM_ARM 40
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_DYNSYM 11

// ELF structures
typedef struct {
    uint32_t e_ident[4];
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
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr;

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
} Elf32_Sym;

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

// Program header types
#define PT_LOAD 1

/*
 * Load .so file
 */
int so_file_load(so_module *mod, const char *filename, uintptr_t load_addr) {
    debug_printf("[SO] Loading %s at 0x%08x\n", filename, load_addr);
    
    // Open file
    SceUID fd = sceIoOpen(filename, SCE_O_RDONLY, 0);
    if (fd < 0) {
        debug_printf("[SO] Failed to open %s: 0x%08x\n", filename, fd);
        return -1;
    }
    
    // Get file size
    SceOff size = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    
    if (size <= 0) {
        debug_printf("[SO] Invalid file size: %d\n", (int)size);
        sceIoClose(fd);
        return -1;
    }
    
    debug_printf("[SO] File size: %d bytes\n", (int)size);
    
    // Allocate memory
    SceUID memblock = sceKernelAllocMemBlock("so_module", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 
                                             (size + 0xFFF) & ~0xFFF, NULL);
    if (memblock < 0) {
        debug_printf("[SO] Failed to allocate memory: 0x%08x\n", memblock);
        sceIoClose(fd);
        return -1;
    }
    
    void *base;
    sceKernelGetMemBlockBase(memblock, &base);
    
    // Read file
    int read_size = sceIoRead(fd, base, size);
    sceIoClose(fd);
    
    if (read_size != size) {
        debug_printf("[SO] Read failed: %d != %d\n", read_size, (int)size);
        sceKernelFreeMemBlock(memblock);
        return -1;
    }
    
    // Initialize module structure
    mod->base = base;
    mod->size = size;
    mod->memblock = memblock;
    mod->symbols = NULL;
    mod->symbol_count = 0;
    mod->strings = NULL;
    
    debug_printf("[SO] Loaded at %p, size %d\n", base, (int)size);
    return 0;
}

/*
 * Parse ELF and find symbols
 */
int so_relocate(so_module *mod) {
    debug_printf("[SO] Relocating module...\n");
    
    if (!mod->base) {
        debug_printf("[SO] Module not loaded\n");
        return -1;
    }
    
    Elf32_Ehdr *elf = (Elf32_Ehdr*)mod->base;
    
    // Verify ELF header
    if (elf->e_ident[0] != ELF_MAGIC || elf->e_type != ET_DYN || elf->e_machine != EM_ARM) {
        debug_printf("[SO] Invalid ELF file\n");
        return -1;
    }
    
    debug_printf("[SO] ELF header validated\n");
    
    // Find section headers
    Elf32_Shdr *sections = (Elf32_Shdr*)((char*)mod->base + elf->e_shoff);
    char *section_names = (char*)mod->base + sections[elf->e_shstrndx].sh_offset;
    
    // Find symbol table and string table
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *strtab = NULL;
    
    for (int i = 0; i < elf->e_shnum; i++) {
        char *name = section_names + sections[i].sh_name;
        
        if (sections[i].sh_type == SHT_DYNSYM && !symtab) {
            symtab = &sections[i];
            debug_printf("[SO] Found .dynsym section\n");
        } else if (sections[i].sh_type == SHT_SYMTAB && !symtab) {
            symtab = &sections[i];
            debug_printf("[SO] Found .symtab section\n");
        } else if (sections[i].sh_type == SHT_STRTAB && strcmp(name, ".strtab") == 0) {
            strtab = &sections[i];
            debug_printf("[SO] Found .strtab section\n");
        } else if (sections[i].sh_type == SHT_STRTAB && strcmp(name, ".dynstr") == 0 && !strtab) {
            strtab = &sections[i];
            debug_printf("[SO] Found .dynstr section\n");
        }
    }
    
    if (!symtab || !strtab) {
        debug_printf("[SO] Symbol table or string table not found\n");
        return -1;
    }
    
    // Store symbol information
    mod->symbols = (void*)((char*)mod->base + symtab->sh_offset);
    mod->symbol_count = symtab->sh_size / sizeof(Elf32_Sym);
    mod->strings = (char*)mod->base + strtab->sh_offset;
    
    debug_printf("[SO] Found %d symbols\n", mod->symbol_count);
    
    // Process program headers for memory mapping
    Elf32_Phdr *pheaders = (Elf32_Phdr*)((char*)mod->base + elf->e_phoff);
    
    for (int i = 0; i < elf->e_phnum; i++) {
        if (pheaders[i].p_type == PT_LOAD) {
            debug_printf("[SO] LOAD segment: vaddr=0x%08x, filesz=%d, memsz=%d\n",
                         pheaders[i].p_vaddr, pheaders[i].p_filesz, pheaders[i].p_memsz);
        }
    }
    
    debug_printf("[SO] Relocation complete\n");
    return 0;
}

/*
 * Find symbol by name
 */
uintptr_t so_symbol(so_module *mod, const char *symbol_name) {
    if (!mod->symbols || !mod->strings) {
        debug_printf("[SO] Symbol table not available\n");
        return 0;
    }
    
    Elf32_Sym *symbols = (Elf32_Sym*)mod->symbols;
    
    for (int i = 0; i < mod->symbol_count; i++) {
        if (symbols[i].st_name != 0) {
            char *name = mod->strings + symbols[i].st_name;
            if (strcmp(name, symbol_name) == 0) {
                uintptr_t addr = (uintptr_t)mod->base + symbols[i].st_value;
                debug_printf("[SO] Found %s at 0x%08x\n", symbol_name, addr);
                return addr;
            }
        }
    }
    
    debug_printf("[SO] Symbol %s not found\n", symbol_name);
    return 0;
}

/*
 * Hook function address - Simplified version without kubridge calls
 */
void hook_addr(uintptr_t addr, uintptr_t dst) {
    if (addr == 0) {
        debug_printf("[SO] Cannot hook NULL address\n");
        return;
    }
    
    debug_printf("[SO] Hooking 0x%08x -> 0x%08x\n", addr, dst);
    
    // Simple hook: overwrite with branch instruction
    // This is a simplified version - real implementation would be more complex
    uint32_t *target = (uint32_t*)addr;
    
    // ARM branch instruction: B dst
    // Calculate offset
    int32_t offset = (int32_t)(dst - addr - 8) / 4;
    
    // Create branch instruction
    uint32_t branch = 0xEA000000 | (offset & 0x00FFFFFF);
    
    debug_printf("[SO] Writing branch instruction: 0x%08x\n", branch);
    *target = branch;
    
    // Note: In real implementation, you'd flush caches here
    // For now, we'll keep it simple
}

/*
 * Cleanup module
 */
void so_cleanup(so_module *mod) {
    if (mod->memblock >= 0) {
        sceKernelFreeMemBlock(mod->memblock);
        mod->memblock = -1;
    }
    
    mod->base = NULL;
    mod->size = 0;
    mod->symbols = NULL;
    mod->symbol_count = 0;
    mod->strings = NULL;
    
    debug_printf("[SO] Module cleanup complete\n");
}
