/*
 * fios.c - File I/O management system for Fluffy Diver
 * Based on GTA SA Vita FIOS methodology
 * Reference: https://github.com/TheOfficialFloW/conduit_vita/blob/master/loader/fios.c
 */

#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "fios.h"

// FIOS configuration
#define FIOS_BUFFER_SIZE (256 * 1024)  // 256KB buffer
#define MAX_PATH_REDIRECTS 32

// Path redirect structure
typedef struct {
    char from[256];
    char to[256];
} PathRedirect;

// FIOS state
static int fios_initialized = 0;
static void *fios_buffer = NULL;
static PathRedirect path_redirects[MAX_PATH_REDIRECTS];
static int num_redirects = 0;

// Asset path mappings for Fluffy Diver
static const PathRedirect default_redirects[] = {
    // Android asset paths -> Vita paths
    {"/android_asset/", "ux0:data/fluffydiver/assets/"},
    {"assets/", "ux0:data/fluffydiver/assets/"},
    {"/sdcard/Android/data/com.hotdog.fluffydiver/", "ux0:data/fluffydiver/"},
    {"/data/data/com.hotdog.fluffydiver/", "ux0:data/fluffydiver/"},
    {"/storage/emulated/0/Android/data/com.hotdog.fluffydiver/", "ux0:data/fluffydiver/"},

    // Common Android paths
    {"/sdcard/", "ux0:data/fluffydiver/sdcard/"},
    {"/storage/emulated/0/", "ux0:data/fluffydiver/sdcard/"},
    {"/data/", "ux0:data/fluffydiver/data/"},
    {"/cache/", "ux0:data/fluffydiver/cache/"},

    // Game-specific paths
    {"com.hotdog.fluffydiver", "fluffydiver"},
    {"HotDog", "fluffydiver"},
    {"hotdog", "fluffydiver"},
};

// Initialize FIOS system
int fios_init(void) {
    if (fios_initialized) {
        printf("FIOS: Already initialized\n");
        return 1;
    }

    printf("FIOS: Initializing file I/O system...\n");

    // Allocate buffer for FIOS operations
    fios_buffer = malloc(FIOS_BUFFER_SIZE);
    if (!fios_buffer) {
        printf("FIOS: ERROR - Failed to allocate buffer\n");
        return -1;
    }

    // Initialize path redirects
    num_redirects = 0;

    // Add default redirects
    int default_count = sizeof(default_redirects) / sizeof(default_redirects[0]);
    for (int i = 0; i < default_count && num_redirects < MAX_PATH_REDIRECTS; i++) {
        strcpy(path_redirects[num_redirects].from, default_redirects[i].from);
        strcpy(path_redirects[num_redirects].to, default_redirects[i].to);
        num_redirects++;
    }

    printf("FIOS: Added %d default path redirects\n", num_redirects);

    // Create necessary directories
    fios_create_directories();

    fios_initialized = 1;
    printf("FIOS: Initialization complete\n");
    return 0;
}

// Create necessary directories for the game
void fios_create_directories(void) {
    printf("FIOS: Creating directory structure...\n");

    const char *directories[] = {
        "ux0:data/fluffydiver",
        "ux0:data/fluffydiver/assets",
        "ux0:data/fluffydiver/data",
        "ux0:data/fluffydiver/cache",
        "ux0:data/fluffydiver/sdcard",
        "ux0:data/fluffydiver/save",
        "ux0:data/fluffydiver/logs",
        NULL
    };

    for (int i = 0; directories[i] != NULL; i++) {
        sceIoMkdir(directories[i], 0777);
        printf("FIOS: Created directory: %s\n", directories[i]);
    }
}

// Translate Android path to Vita path
char *fios_translate_path(const char *path) {
    if (!path) {
        return NULL;
    }

    static char translated_path[512];
    strcpy(translated_path, path);

    // Apply path redirects
    for (int i = 0; i < num_redirects; i++) {
        const char *from = path_redirects[i].from;
        const char *to = path_redirects[i].to;

        char *match = strstr(translated_path, from);
        if (match == translated_path) { // Must match at beginning
            // Replace the path
            char temp[512];
            strcpy(temp, to);
            strcat(temp, path + strlen(from));
            strcpy(translated_path, temp);
            break;
        }
    }

    // Convert forward slashes to appropriate separators if needed
    // (Vita handles forward slashes fine, so we don't need to convert)

    return translated_path;
}

// Add custom path redirect
int fios_add_redirect(const char *from, const char *to) {
    if (num_redirects >= MAX_PATH_REDIRECTS) {
        printf("FIOS: ERROR - Maximum path redirects reached\n");
        return -1;
    }

    strcpy(path_redirects[num_redirects].from, from);
    strcpy(path_redirects[num_redirects].to, to);
    num_redirects++;

    printf("FIOS: Added redirect: %s -> %s\n", from, to);
    return 0;
}

// Check if file exists (with path translation)
int fios_file_exists(const char *path) {
    char *translated = fios_translate_path(path);

    SceIoStat stat;
    int result = sceIoGetstat(translated, &stat) >= 0;

    if (result) {
        printf("FIOS: File exists: %s -> %s\n", path, translated);
    }

    return result;
}

// Get file size (with path translation)
long fios_file_size(const char *path) {
    char *translated = fios_translate_path(path);

    SceIoStat stat;
    if (sceIoGetstat(translated, &stat) < 0) {
        return -1;
    }

    return stat.st_size;
}

// Copy file with path translation
int fios_copy_file(const char *src, const char *dst) {
    char *src_translated = fios_translate_path(src);
    char *dst_translated = fios_translate_path(dst);

    printf("FIOS: Copying file: %s -> %s\n", src_translated, dst_translated);

    SceUID src_fd = sceIoOpen(src_translated, SCE_O_RDONLY, 0);
    if (src_fd < 0) {
        printf("FIOS: ERROR - Cannot open source file: 0x%08X\n", src_fd);
        return -1;
    }

    SceUID dst_fd = sceIoOpen(dst_translated, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (dst_fd < 0) {
        printf("FIOS: ERROR - Cannot create destination file: 0x%08X\n", dst_fd);
        sceIoClose(src_fd);
        return -1;
    }

    // Copy data using our buffer
    int bytes_copied = 0;
    int bytes_read;

    while ((bytes_read = sceIoRead(src_fd, fios_buffer, FIOS_BUFFER_SIZE)) > 0) {
        int bytes_written = sceIoWrite(dst_fd, fios_buffer, bytes_read);
        if (bytes_written != bytes_read) {
            printf("FIOS: ERROR - Write failed\n");
            sceIoClose(src_fd);
            sceIoClose(dst_fd);
            return -1;
        }
        bytes_copied += bytes_written;
    }

    sceIoClose(src_fd);
    sceIoClose(dst_fd);

    printf("FIOS: File copied successfully, %d bytes\n", bytes_copied);
    return bytes_copied;
}

// Create directory with path translation
int fios_mkdir(const char *path) {
    char *translated = fios_translate_path(path);

    int result = sceIoMkdir(translated, 0777);
    if (result < 0 && result != SCE_ERROR_ERRNO_EEXIST) {
        printf("FIOS: ERROR - Cannot create directory %s: 0x%08X\n", translated, result);
        return -1;
    }

    printf("FIOS: Directory created: %s\n", translated);
    return 0;
}

// Remove file with path translation
int fios_remove(const char *path) {
    char *translated = fios_translate_path(path);

    int result = sceIoRemove(translated);
    if (result < 0) {
        printf("FIOS: ERROR - Cannot remove file %s: 0x%08X\n", translated, result);
        return -1;
    }

    printf("FIOS: File removed: %s\n", translated);
    return 0;
}

// List directory contents
int fios_list_directory(const char *path) {
    char *translated = fios_translate_path(path);

    printf("FIOS: Listing directory: %s -> %s\n", path, translated);

    SceUID dir = sceIoDopen(translated);
    if (dir < 0) {
        printf("FIOS: ERROR - Cannot open directory: 0x%08X\n", dir);
        return -1;
    }

    SceIoDirent entry;
    int count = 0;

    while (sceIoDread(dir, &entry) > 0) {
        printf("FIOS:   %s %s\n",
               SCE_S_ISDIR(entry.d_stat.st_mode) ? "[DIR]" : "[FILE]",
               entry.d_name);
        count++;
    }

    sceIoDclose(dir);
    printf("FIOS: Directory contains %d entries\n", count);
    return count;
}

// Get free space on device
long long fios_get_free_space(const char *path) {
    char *translated = fios_translate_path(path);

    SceIoDevInfo info;
    int result = sceIoDevctl(translated, 0x3001, NULL, 0, &info, sizeof(info));
    if (result < 0) {
        printf("FIOS: ERROR - Cannot get device info: 0x%08X\n", result);
        return -1;
    }

    long long free_space = (long long)info.free_size * info.cluster_size;
    printf("FIOS: Free space on device: %lld bytes\n", free_space);

    return free_space;
}

// Print current path redirects (for debugging)
void fios_print_redirects(void) {
    printf("FIOS: Current path redirects (%d):\n", num_redirects);
    for (int i = 0; i < num_redirects; i++) {
        printf("FIOS:   %s -> %s\n", path_redirects[i].from, path_redirects[i].to);
    }
}

// Cleanup FIOS system
void fios_cleanup(void) {
    if (!fios_initialized) {
        return;
    }

    if (fios_buffer) {
        free(fios_buffer);
        fios_buffer = NULL;
    }

    num_redirects = 0;
    fios_initialized = 0;

    printf("FIOS: System cleaned up\n");
}

// Enhanced file operations for Android compatibility

// Android-style fopen wrapper
FILE *fios_fopen(const char *path, const char *mode) {
    char *translated = fios_translate_path(path);

    FILE *file = fopen(translated, mode);
    if (file) {
        printf("FIOS: Opened file: %s -> %s (mode: %s)\n", path, translated, mode);
    } else {
        printf("FIOS: Failed to open file: %s -> %s (mode: %s)\n", path, translated, mode);
    }

    return file;
}

// Android asset manager style access
int fios_asset_exists(const char *asset_path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "assets/%s", asset_path);
    return fios_file_exists(full_path);
}

long fios_asset_size(const char *asset_path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "assets/%s", asset_path);
    return fios_file_size(full_path);
}

FILE *fios_asset_open(const char *asset_path, const char *mode) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "assets/%s", asset_path);
    return fios_fopen(full_path, mode);
}
