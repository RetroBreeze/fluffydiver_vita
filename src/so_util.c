#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <kubridge.h>

#include "so_util.h"

// External reference to module
extern so_module fluffydiver_mod;

// External debug function
extern void debugPrintf(const char *fmt, ...);

// Simplified ELF structures
#define PT_LOAD 1
#define DT_NULL 0
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_PLTRELSZ 2
#define DT_JMPREL 23
#define DT_INIT_ARRAY 25
#define DT_INIT_ARRAYSZ 27

// Relocation types
#define R_ARM_NONE 0
#define R_ARM_PC24 1
#define R_ARM_ABS32 2
#define R_ARM_REL32 3
#define R_ARM_GLOB_DAT 21
#define R_ARM_JUMP_SLOT 22
#define R_ARM_RELATIVE 23

// Memory mapping flags
#define SCE_KERNEL_MAP_FIXED 0x00000010

// ELF structures
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

typedef struct {
    uint32_t d_tag;
    uint32_t d_val;
} Elf32_Dyn;

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
} Elf32_Sym;

typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;

int so_load(so_module *mod, const char *path, uintptr_t load_addr) {
    debugPrintf("[SO] so_load called with path: %s, addr: 0x%08X\n", path, load_addr);

    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) {
        debugPrintf("[SO] ERROR: Failed to open file: 0x%08X\n", fd);
        return -1;
    }
    debugPrintf("[SO] File opened successfully, fd: %d\n", fd);

    // Read ELF header
    char ehdr[52];
    int read_size = sceIoRead(fd, ehdr, 52);
    if (read_size != 52) {
        debugPrintf("[SO] ERROR: Failed to read ELF header, got %d bytes\n", read_size);
        sceIoClose(fd);
        return -1;
    }
    debugPrintf("[SO] Read ELF header: %d bytes\n", read_size);

    // Check ELF magic
    if (*(uint32_t*)ehdr != 0x464C457F) {
        debugPrintf("[SO] ERROR: Invalid ELF magic: 0x%08X\n", *(uint32_t*)ehdr);
        sceIoClose(fd);
        return -1;
    }
    debugPrintf("[SO] ELF magic verified\n");

    uint32_t phoff = *(uint32_t*)(ehdr + 28);
    uint16_t phnum = *(uint16_t*)(ehdr + 44);
    debugPrintf("[SO] Program headers: offset=0x%08X, count=%d\n", phoff, phnum);

    // Find total size needed
    size_t total_size = 0;
    Elf32_Phdr phdr;

    for (int i = 0; i < phnum; i++) {
        sceIoLseek(fd, phoff + i * sizeof(Elf32_Phdr), SCE_SEEK_SET);
        sceIoRead(fd, &phdr, sizeof(phdr));

        if (phdr.p_type == PT_LOAD) {
            size_t end = phdr.p_vaddr + phdr.p_memsz;
            if (end > total_size) total_size = end;
            debugPrintf("[SO] PT_LOAD segment %d: vaddr=0x%08X, memsz=0x%08X\n",
                        i, phdr.p_vaddr, phdr.p_memsz);
        }
    }

    debugPrintf("[SO] Total size needed: 0x%08X bytes\n", total_size);

    // Allocate memory - for now use dynamic allocation
    // TODO: Use kubridge for fixed address mapping like GTA SA Vita
    debugPrintf("[SO] Allocating %d bytes...\n", (total_size + 0xFFF) & ~0xFFF);
    mod->size = (total_size + 0xFFF) & ~0xFFF;

    // Allocate a memory block
    SceUID memblock = sceKernelAllocMemBlock("so_module",
                                             SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
                                             mod->size, NULL);
    if (memblock < 0) {
        debugPrintf("[SO] ERROR: sceKernelAllocMemBlock failed: 0x%08X\n", memblock);
        sceIoClose(fd);
        return -1;
    }

    // Get the base address
    void *mem_base;
    sceKernelGetMemBlockBase(memblock, &mem_base);
    mod->base = mem_base;
    debugPrintf("[SO] Memory allocated at %p (size: 0x%08X)\n", mod->base, mod->size);

    // Load segments
    debugPrintf("[SO] Loading segments...\n");
    for (int i = 0; i < phnum; i++) {
        sceIoLseek(fd, phoff + i * sizeof(Elf32_Phdr), SCE_SEEK_SET);
        sceIoRead(fd, &phdr, sizeof(phdr));

        if (phdr.p_type == PT_LOAD) {
            void *dst = (char*)mod->base + phdr.p_vaddr;

            debugPrintf("[SO] Loading segment %d to %p (filesz=0x%X, memsz=0x%X)\n",
                        i, dst, phdr.p_filesz, phdr.p_memsz);

            if (phdr.p_filesz > 0) {
                sceIoLseek(fd, phdr.p_offset, SCE_SEEK_SET);
                int bytes_read = sceIoRead(fd, dst, phdr.p_filesz);
                if (bytes_read != phdr.p_filesz) {
                    debugPrintf("[SO] ERROR: Read failed, expected %d got %d\n",
                                phdr.p_filesz, bytes_read);
                }
            }

            if (phdr.p_memsz > phdr.p_filesz) {
                sceClibMemset((char*)dst + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
            }
        }
    }

    sceIoClose(fd);
    debugPrintf("[SO] Load complete\n");
    return 0;
}

int so_relocate(so_module *mod) {
    debugPrintf("[SO] Relocating module...\n");

    // Find dynamic section
    char *ehdr = (char*)mod->base;
    uint32_t phoff = *(uint32_t*)(ehdr + 28);
    uint16_t phnum = *(uint16_t*)(ehdr + 44);

    Elf32_Phdr *phdrs = (Elf32_Phdr*)((char*)mod->base + phoff);
    Elf32_Dyn *dynamic = NULL;

    // Find PT_DYNAMIC segment
    for (int i = 0; i < phnum; i++) {
        if (phdrs[i].p_type == 2) { // PT_DYNAMIC
            dynamic = (Elf32_Dyn*)((char*)mod->base + phdrs[i].p_vaddr);
            debugPrintf("[SO] Found PT_DYNAMIC at offset 0x%08X\n", phdrs[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) {
        debugPrintf("[SO] ERROR: No PT_DYNAMIC segment found\n");
        return -1;
    }

    // Parse dynamic section
    int found_symtab = 0, found_strtab = 0, found_hash = 0;
    int found_rel = 0, found_plt_rel = 0;
    Elf32_Dyn *dyn = dynamic;
    while (dyn->d_tag != DT_NULL) {
        switch (dyn->d_tag) {
            case DT_SYMTAB:
                mod->dynsym = (char*)mod->base + dyn->d_val;
                debugPrintf("[SO] Found DT_SYMTAB at 0x%08X\n", dyn->d_val);
                found_symtab = 1;
                break;
            case DT_STRTAB:
                mod->dynstr = (char*)mod->base + dyn->d_val;
                debugPrintf("[SO] Found DT_STRTAB at 0x%08X\n", dyn->d_val);
                found_strtab = 1;
                break;
            case DT_HASH:
                mod->hash = (char*)mod->base + dyn->d_val;
                debugPrintf("[SO] Found DT_HASH at 0x%08X\n", dyn->d_val);
                found_hash = 1;
                break;
            case DT_REL:
                mod->rel = (char*)mod->base + dyn->d_val;
                debugPrintf("[SO] Found DT_REL at 0x%08X\n", dyn->d_val);
                found_rel = 1;
                break;
            case DT_RELSZ:
                mod->rel_size = dyn->d_val;
                debugPrintf("[SO] Found DT_RELSZ: %d bytes\n", dyn->d_val);
                break;
            case DT_JMPREL:
                mod->plt_rel = (char*)mod->base + dyn->d_val;
                debugPrintf("[SO] Found DT_JMPREL at 0x%08X\n", dyn->d_val);
                found_plt_rel = 1;
                break;
            case DT_PLTRELSZ:
                mod->plt_rel_size = dyn->d_val;
                debugPrintf("[SO] Found DT_PLTRELSZ: %d bytes\n", dyn->d_val);
                break;
        }
        dyn++;
    }

    debugPrintf("[SO] Dynamic parsing complete: symtab=%d, strtab=%d, hash=%d, rel=%d, plt_rel=%d\n",
                found_symtab, found_strtab, found_hash, found_rel, found_plt_rel);

    return 0;
}

int so_resolve(so_module *mod, DynLibFunction *funcs, size_t num_funcs, int strict) {
    debugPrintf("[SO] Starting symbol resolution for %d functions\n", (int)num_funcs);

    if (!mod->dynsym || !mod->dynstr) {
        debugPrintf("[SO] ERROR: No symbol/string table found\n");
        return -1;
    }

    Elf32_Sym *syms = (Elf32_Sym*)mod->dynsym;
    int resolved_count = 0;
    int unresolved_count = 0;

    // Process PLT relocations (function calls)
    if (mod->plt_rel && mod->plt_rel_size > 0) {
        debugPrintf("[SO] Processing %d PLT relocations\n", mod->plt_rel_size / sizeof(Elf32_Rel));
        Elf32_Rel *plt_rel = (Elf32_Rel*)mod->plt_rel;
        int plt_count = mod->plt_rel_size / sizeof(Elf32_Rel);

        for (int i = 0; i < plt_count; i++) {
            uint32_t sym_idx = plt_rel[i].r_info >> 8;
            uint32_t type = plt_rel[i].r_info & 0xFF;

            if (type == R_ARM_JUMP_SLOT && sym_idx != 0) {
                const char *name = (char*)mod->dynstr + syms[sym_idx].st_name;
                uint32_t *got_entry = (uint32_t*)((char*)mod->base + plt_rel[i].r_offset);

                // Search for this symbol in our default_dynlib
                uintptr_t func_addr = 0;
                for (size_t j = 0; j < num_funcs; j++) {
                    if (strcmp(name, funcs[j].symbol) == 0) {
                        func_addr = funcs[j].func;
                        break;
                    }
                }

                if (func_addr != 0) {
                    // Patch the GOT entry with our function address
                    *got_entry = func_addr;
                    debugPrintf("[SO] Resolved: %s -> 0x%08X (GOT: %p)\n", name, func_addr, got_entry);
                    resolved_count++;
                } else {
                    debugPrintf("[SO] Unresolved: %s\n", name);
                    unresolved_count++;
                    if (strict) {
                        debugPrintf("[SO] ERROR: Required symbol %s not found\n", name);
                        return -1;
                    }
                }
            }
        }
    }

    // Process REL relocations (data references)
    if (mod->rel && mod->rel_size > 0) {
        debugPrintf("[SO] Processing %d REL relocations\n", mod->rel_size / sizeof(Elf32_Rel));
        Elf32_Rel *rel = (Elf32_Rel*)mod->rel;
        int rel_count = mod->rel_size / sizeof(Elf32_Rel);

        for (int i = 0; i < rel_count; i++) {
            uint32_t sym_idx = rel[i].r_info >> 8;
            uint32_t type = rel[i].r_info & 0xFF;
            uint32_t *target = (uint32_t*)((char*)mod->base + rel[i].r_offset);

            switch (type) {
                case R_ARM_ABS32:
                    if (sym_idx != 0) {
                        const char *name = (char*)mod->dynstr + syms[sym_idx].st_name;

                        // Search for this symbol in our default_dynlib
                        uintptr_t func_addr = 0;
                        for (size_t j = 0; j < num_funcs; j++) {
                            if (strcmp(name, funcs[j].symbol) == 0) {
                                func_addr = funcs[j].func;
                                break;
                            }
                        }

                        if (func_addr != 0) {
                            *target = func_addr;
                            resolved_count++;
                        } else {
                            unresolved_count++;
                        }
                    }
                    break;

                case R_ARM_RELATIVE:
                    // Adjust by base address
                    *target += (uint32_t)mod->base;
                    break;

                case R_ARM_GLOB_DAT:
                    if (sym_idx != 0) {
                        const char *name = (char*)mod->dynstr + syms[sym_idx].st_name;

                        // Search for this symbol
                        uintptr_t func_addr = 0;
                        for (size_t j = 0; j < num_funcs; j++) {
                            if (strcmp(name, funcs[j].symbol) == 0) {
                                func_addr = funcs[j].func;
                                break;
                            }
                        }

                        if (func_addr != 0) {
                            *target = func_addr;
                            resolved_count++;
                        }
                    }
                    break;
            }
        }
    }

    debugPrintf("[SO] Symbol resolution complete: %d resolved, %d unresolved\n",
                resolved_count, unresolved_count);

    return 0;
}

void so_flush_caches(so_module *mod) {
    debugPrintf("[SO] Flushing caches for range %p - %p\n",
                mod->base, (char*)mod->base + mod->size);
    kuKernelFlushCaches(mod->base, mod->size);
}

int so_initialize(so_module *mod) {
    debugPrintf("[SO] Initializing module...\n");

    // Find dynamic section to look for init functions
    char *ehdr = (char*)mod->base;
    uint32_t phoff = *(uint32_t*)(ehdr + 28);
    uint16_t phnum = *(uint16_t*)(ehdr + 44);

    Elf32_Phdr *phdrs = (Elf32_Phdr*)((char*)mod->base + phoff);
    Elf32_Dyn *dynamic = NULL;

    // Find PT_DYNAMIC segment
    for (int i = 0; i < phnum; i++) {
        if (phdrs[i].p_type == 2) { // PT_DYNAMIC
            dynamic = (Elf32_Dyn*)((char*)mod->base + phdrs[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) {
        debugPrintf("[SO] No PT_DYNAMIC segment found\n");
        return 0;
    }

    // Look for init functions
    Elf32_Dyn *dyn = dynamic;
    while (dyn->d_tag != DT_NULL) {
        switch (dyn->d_tag) {
            case DT_INIT: {
                void (*init_func)(void) = (void*)((char*)mod->base + dyn->d_val);
                debugPrintf("[SO] Calling DT_INIT at 0x%08X\n", (uint32_t)init_func);
                init_func();
                break;
            }
            case DT_INIT_ARRAY: {
                void **init_array_ptr = (void**)((char*)mod->base + dyn->d_val);
                debugPrintf("[SO] Found DT_INIT_ARRAY at 0x%08X\n", dyn->d_val);
                // Note: Processing init_array requires DT_INIT_ARRAYSZ to know count
                // GTA SA Vita approach: Log but don't execute here
                (void)init_array_ptr; // Suppress unused variable warning
                break;
            }
            case DT_INIT_ARRAYSZ: {
                // This would be handled with DT_INIT_ARRAY
                debugPrintf("[SO] DT_INIT_ARRAYSZ: %d bytes\n", dyn->d_val);
                break;
            }
        }
        dyn++;
    }

    debugPrintf("[SO] Module initialization complete\n");
    return 0;
}

uintptr_t so_symbol(so_module *mod, const char *symbol_name) {
    if (!mod->dynsym || !mod->dynstr || !mod->hash) return 0;

    // Simple linear search - real implementation uses hash table
    uint32_t *hash = (uint32_t*)mod->hash;
    uint32_t nchain = hash[1];

    Elf32_Sym *syms = (Elf32_Sym*)mod->dynsym;

    for (uint32_t i = 0; i < nchain; i++) {
        const char *name = (char*)mod->dynstr + syms[i].st_name;
        if (strcmp(name, symbol_name) == 0) {
            return (uintptr_t)mod->base + syms[i].st_value;
        }
    }

    return 0;
}

void hook_addr(uintptr_t addr, uintptr_t dst) {
    if (addr == 0) {
        debugPrintf("[SO] WARNING: Trying to hook NULL address\n");
        return;
    }

    // Write a branch instruction to our destination
    // ARM branch instruction: 0xEA000000 | ((offset >> 2) & 0x00FFFFFF)
    uint32_t offset = dst - addr - 8; // Account for pipeline
    uint32_t branch = 0xEA000000 | ((offset >> 2) & 0x00FFFFFF);

    // Use kubridge to write to protected memory
    kuKernelCpuUnrestrictedMemcpy((void*)addr, &branch, 4);

    debugPrintf("[SO] Hooked: 0x%08X -> 0x%08X (branch: 0x%08X)\n", addr, dst, branch);
}

int ret0() {
    return 0;
}

int ret1() {
    return 1;
}

int retminus1() {
    return -1;
}

void *retNULL() {
    return NULL;
}

// Comprehensive symbol analysis and execution function
int so_analyze_and_try_symbols(so_module *mod, void *fake_env, void *fake_context) {
    debugPrintf("=== COMPREHENSIVE SYMBOL ANALYSIS ===\n");

    if (!mod->hash || !mod->dynstr || !mod->dynsym) {
        debugPrintf("ERROR: Symbol table not properly loaded\n");
        return -1;
    }

    uint32_t *hash = (uint32_t*)mod->hash;
    uint32_t nchain = hash[1];
    Elf32_Sym *syms = (Elf32_Sym*)mod->dynsym;

    debugPrintf("Total symbols in library: %d\n", nchain);

    // Look for any symbols that might be entry points
    int java_symbols = 0;
    int native_symbols = 0;
    int jni_symbols = 0;
    int valid_code_symbols = 0;

    for (uint32_t i = 0; i < nchain && i < 100; i++) { // Limit to first 100 symbols
        const char *name = (char*)mod->dynstr + syms[i].st_name;
        if (name && strlen(name) > 0) {
            debugPrintf("Symbol %d: %s (value: 0x%08X)\n", i, name, syms[i].st_value);

            if (strstr(name, "Java_")) java_symbols++;
            if (strstr(name, "native")) native_symbols++;
            if (strstr(name, "JNI")) jni_symbols++;

            // Check if this symbol points to executable code
            if (syms[i].st_value != 0) {
                uintptr_t sym_addr = (uintptr_t)mod->base + syms[i].st_value;

                // Ensure address is within module bounds
                if (sym_addr >= (uintptr_t)mod->base &&
                    sym_addr < (uintptr_t)mod->base + mod->size) {

                    uint32_t *code_ptr = (uint32_t*)sym_addr;
                uint32_t first_inst = code_ptr[0];

                // Check for ARM patterns
                if ((first_inst & 0xffff0000) == 0xe92d0000) {
                    debugPrintf("  ✓ %s has ARM prologue - trying as entry point!\n", name);
                    valid_code_symbols++;

                    // Try this as an entry point
                    debugPrintf("  Attempting to call %s as entry point...\n", name);
                    typedef void (*potential_entry_t)(void* env, void* thiz);
                    potential_entry_t potential_func = (potential_entry_t)sym_addr;

                    debugPrintf("  Calling %s...\n", name);
                    potential_func(fake_env, fake_context);
                    debugPrintf("  ✓ %s succeeded!\n", name);
                    return 0; // Success!

                } else {
                    // Check for Thumb patterns
                    uint16_t *thumb_ptr = (uint16_t*)sym_addr;
                    uint16_t thumb_inst = thumb_ptr[0];

                    if ((thumb_inst & 0xFF00) == 0xB500) {
                        debugPrintf("  ✓ %s has Thumb prologue - trying as entry point!\n", name);
                        valid_code_symbols++;

                        // Try as Thumb (set LSB)
                        typedef void (*thumb_entry_t)(void* env, void* thiz);
                        thumb_entry_t thumb_func = (thumb_entry_t)(sym_addr | 1);

                        debugPrintf("  Calling %s in Thumb mode...\n", name);
                        thumb_func(fake_env, fake_context);
                        debugPrintf("  ✓ %s (Thumb) succeeded!\n", name);
                        return 0; // Success!
                    }
                }
                    } else {
                        debugPrintf("  WARNING: %s address 0x%08X is outside module bounds\n", name, sym_addr);
                    }
            }
        }
    }

    debugPrintf("Symbol analysis complete:\n");
    debugPrintf("- Java symbols: %d\n", java_symbols);
    debugPrintf("- Native symbols: %d\n", native_symbols);
    debugPrintf("- JNI symbols: %d\n", jni_symbols);
    debugPrintf("- Valid code symbols: %d\n", valid_code_symbols);

    if (java_symbols == 0 && jni_symbols == 0) {
        debugPrintf("WARNING: No Java/JNI symbols found - this may not be an Android native library!\n");
    }

    if (valid_code_symbols == 0) {
        debugPrintf("ERROR: No symbols with valid ARM/Thumb code found!\n");
    }

    debugPrintf("=== FINAL DIAGNOSIS ===\n");
    debugPrintf("This appears to be an incompatible or corrupted library.\n");
    debugPrintf("Possible issues:\n");
    debugPrintf("1. Wrong architecture (x86/x64 instead of ARM)\n");
    debugPrintf("2. Hardcoded addresses that don't work with so-loader\n");
    debugPrintf("3. Missing dependencies or initialization requirements\n");
    debugPrintf("4. Library requires specific Android framework components\n");
    debugPrintf("5. File corruption during extraction\n");

    return -1; // Failed to find working entry point
}
