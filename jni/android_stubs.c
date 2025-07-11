#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>

/*
 * ANDROID API STUBS WITH ASSET LOGGING
 * These replace Android system calls that the game expects
 */

// Asset logging
static FILE *asset_log = NULL;

static void init_asset_log() {
    asset_log = fopen("ux0:data/fluffydiver/asset_access.log", "w");
    if (asset_log) {
        fprintf(asset_log, "=== Asset Access Log ===\n");
        fprintf(asset_log, "Logging all file access attempts...\n\n");
        fflush(asset_log);
    }
}

static void log_asset_access(const char *filename, const char *mode, int success, const char *actual_path) {
    if (asset_log) {
        fprintf(asset_log, "[ASSET] %s (%s) -> %s", 
                filename, mode, success ? "SUCCESS" : "FAILED");
        if (success && actual_path) {
            fprintf(asset_log, " (found at: %s)", actual_path);
        }
        fprintf(asset_log, "\n");
        fflush(asset_log);
    }
}

// Android Logging (from your strings analysis)
int __android_log_print(int priority, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Convert to printf
    printf("[%s] ", tag ? tag : "GAME");
    vprintf(fmt, args);
    printf("\n");
    
    va_end(args);
    return 0;
}

// Android log priority levels
#define ANDROID_LOG_UNKNOWN    0
#define ANDROID_LOG_DEFAULT    1
#define ANDROID_LOG_VERBOSE    2
#define ANDROID_LOG_DEBUG      3
#define ANDROID_LOG_INFO       4
#define ANDROID_LOG_WARN       5
#define ANDROID_LOG_ERROR      6
#define ANDROID_LOG_FATAL      7
#define ANDROID_LOG_SILENT     8

// File system redirects - SAFER VERSION (no fopen override)
FILE *android_fopen(const char *filename, const char *mode) {
    if (!asset_log) init_asset_log();
    
    char vita_path[512];
    FILE *file = NULL;
    
    // Try multiple path combinations in order of preference
    const char *base_paths[] = {
        "ux0:data/fluffydiver/",           // Direct in data folder
        "ux0:data/fluffydiver/assets/",    // Assets subfolder
        "app0:assets/",                    // VPK assets folder
        "app0:",                           // VPK root
        ""                                 // Original path as-is
    };
    
    // Clean up the filename first
    const char *clean_filename = filename;
    if (strncmp(filename, "assets/", 7) == 0) {
        clean_filename = filename + 7;
    } else if (strncmp(filename, "/android_asset/", 15) == 0) {
        clean_filename = filename + 15;
    } else if (strncmp(filename, "./", 2) == 0) {
        clean_filename = filename + 2;
    }
    
    // Try each base path
    for (int i = 0; i < 5; i++) {
        snprintf(vita_path, sizeof(vita_path), "%s%s", base_paths[i], clean_filename);
        
        file = fopen(vita_path, mode);
        if (file) {
            log_asset_access(filename, mode, 1, vita_path);
            return file;
        }
    }
    
    // Log the failure with all attempted paths
    log_asset_access(filename, mode, 0, NULL);
    if (asset_log) {
        fprintf(asset_log, "    Tried paths:\n");
        for (int i = 0; i < 5; i++) {
            snprintf(vita_path, sizeof(vita_path), "%s%s", base_paths[i], clean_filename);
            fprintf(asset_log, "      %s\n", vita_path);
        }
        fflush(asset_log);
    }
    
    return NULL;
}

// Directory listing function for debugging
void dump_asset_directory() {
    FILE *f = fopen("ux0:data/fluffydiver/directory_listing.txt", "w");
    if (!f) return;
    
    fprintf(f, "=== Directory Listing ===\n");
    
    const char *dirs_to_check[] = {
        "ux0:data/fluffydiver/",
        "ux0:data/fluffydiver/assets/",
        "app0:assets/",
        "app0:"
    };
    
    for (int d = 0; d < 4; d++) {
        fprintf(f, "\n--- Directory: %s ---\n", dirs_to_check[d]);
        
        SceUID dir = sceIoDopen(dirs_to_check[d]);
        if (dir >= 0) {
            SceIoDirent entry;
            int count = 0;
            while (sceIoDread(dir, &entry) > 0) {
                fprintf(f, "%s (%s, %d bytes)\n", 
                       entry.d_name, 
                       SCE_S_ISDIR(entry.d_stat.st_mode) ? "DIR" : "FILE",
                       (int)entry.d_stat.st_size);
                count++;
            }
            sceIoDclose(dir);
            fprintf(f, "Total items: %d\n", count);
        } else {
            fprintf(f, "Directory not accessible (error: 0x%08X)\n", dir);
        }
    }
    
    fclose(f);
}

// Call this from main.c to create the directory listing
void create_asset_debug_files() {
    dump_asset_directory();
    
    // Create a test file to verify file creation works
    FILE *test = fopen("ux0:data/fluffydiver/test_file_creation.txt", "w");
    if (test) {
        fprintf(test, "File creation test successful!\n");
        fclose(test);
    }
}
