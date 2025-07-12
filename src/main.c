/*
 * Fluffy Diver PS Vita Port - CORRECT Implementation
 * Following the exact pattern from TheOfficialFloW's GTA SA Vita
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/rtc.h>

#include <vitaGL.h>
#include <kubridge.h>
#include "so_util.h"

// Load address for the .so
#define LOAD_ADDRESS 0x8000000

// Game module
so_module fluffydiver_mod;

// Debug logging
static FILE *debug_log = NULL;

// JNI environment - SIMPLE VERSION
static void *jni_env[1000];
static int jni_initialized = 0;

// Simple JNI functions
static int jni_GetVersion() {
    printf("[JNI] GetVersion\n");
    return 0x00010006;
}

static void* jni_FindClass(const char *name) {
    printf("[JNI] FindClass: %s\n", name);
    return (void*)0x12345678;
}

static void* jni_GetMethodID(void *clazz, const char *name, const char *sig) {
    printf("[JNI] GetMethodID: %s\n", name);
    return (void*)0x87654321;
}

static void* jni_NewStringUTF(const char *utf) {
    printf("[JNI] NewStringUTF: %s\n", utf);
    return (void*)0x11111111;
}

static void jni_setup() {
    printf("[JNI] Setting up simple JNI environment\n");

    memset(jni_env, 0, sizeof(jni_env));
    jni_env[4] = jni_GetVersion;
    jni_env[6] = jni_FindClass;
    jni_env[33] = jni_GetMethodID;
    jni_env[167] = jni_NewStringUTF;

    jni_initialized = 1;
    printf("[JNI] JNI environment ready at %p\n", jni_env);
}

// Android logging hook
int android_log_print(int priority, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("[ANDROID][%s] ", tag ? tag : "GAME");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    return 0;
}

// File access hook
FILE* vita_fopen_hook(const char *filename, const char *mode) {
    printf("[FILE] Opening: %s\n", filename);

    char vita_path[512];

    // Redirect asset paths
    if (strstr(filename, "assets/") || strstr(filename, "/android_asset/")) {
        const char *asset_name = strstr(filename, "assets/") ?
        filename + 7 : filename + 15;
        snprintf(vita_path, sizeof(vita_path), "ux0:data/fluffydiver/%s", asset_name);
    } else {
        snprintf(vita_path, sizeof(vita_path), "ux0:data/fluffydiver/%s", filename);
    }

    FILE *file = fopen(vita_path, mode);
    if (file) {
        printf("[FILE] Success: %s\n", vita_path);
    } else {
        printf("[FILE] Failed: %s\n", vita_path);
    }

    return file;
}

// JNI environment getter
void* GetJNIEnv() {
    return jni_env;
}

static void init_debug_log() {
    sceIoMkdir("ux0:data/fluffydiver", 0777);
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");
    if (debug_log) {
        fprintf(debug_log, "=== Fluffy Diver - CORRECT Implementation ===\n");
        fprintf(debug_log, "Following TheOfficialFloW's proven methodology\n\n");
        fflush(debug_log);
    }
}

void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    if (debug_log) {
        vfprintf(debug_log, fmt, args);
        fflush(debug_log);
    }
    va_end(args);
}

static void init_vita_systems() {
    debug_printf("Initializing Vita systems...\n");

    vglInit(0x1000000);
    vglWaitVblankStart(GL_FALSE);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 0);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debug_printf("Vita systems initialized\n");
}

void patch_game() {
    debug_printf("Patching game functions...\n");

    // Hook JNI environment getter
    hook_addr(so_symbol(&fluffydiver_mod, "JNI_GetEnv"), (uintptr_t)GetJNIEnv);

    // Hook Android logging
    uintptr_t android_log = so_symbol(&fluffydiver_mod, "__android_log_print");
    if (android_log) {
        hook_addr(android_log, (uintptr_t)android_log_print);
    }

    // Hook file operations
    uintptr_t fopen_sym = so_symbol(&fluffydiver_mod, "fopen");
    if (fopen_sym) {
        hook_addr(fopen_sym, (uintptr_t)vita_fopen_hook);
    }

    // Set game flags
    uintptr_t debug_flag = so_symbol(&fluffydiver_mod, "debug_mode");
    if (debug_flag) {
        *(int*)debug_flag = 1;
    }

    debug_printf("Game patching complete\n");
}

int main() {
    init_debug_log();
    debug_printf("=== Fluffy Diver PS Vita Port - CORRECT Method ===\n");
    debug_printf("Following proven TheOfficialFloW methodology\n");

    // Initialize Vita systems
    init_vita_systems();

    // Setup JNI environment
    jni_setup();

    // Load .so file
    debug_printf("Loading libFluffyDiver.so...\n");
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    int ret = so_file_load(&fluffydiver_mod, "app0:lib/libFluffyDiver.so", LOAD_ADDRESS);
    if (ret < 0) {
        debug_printf("FATAL: Failed to load .so file\n");
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }

    // Relocate symbols
    debug_printf("Relocating symbols...\n");
    ret = so_relocate(&fluffydiver_mod);
    if (ret < 0) {
        debug_printf("FATAL: Failed to relocate symbols\n");
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }

    // Patch game functions
    debug_printf("Patching game...\n");
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    patch_game();

    // Find and call android_main
    debug_printf("Looking for android_main...\n");

    // Try different entry points
    const char* entry_points[] = {
        "android_main",
        "Java_com_hotdog_jni_Natives_onHotDogCreate",
        "ANativeActivity_onCreate",
        NULL
    };

    void (*android_main)(void) = NULL;

    for (int i = 0; entry_points[i]; i++) {
        android_main = (void(*)(void))so_symbol(&fluffydiver_mod, entry_points[i]);
        if (android_main) {
            debug_printf("Found entry point: %s at %p\n", entry_points[i], android_main);
            break;
        }
    }

    if (android_main) {
        debug_printf("Starting game...\n");
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        // Call android_main - this is the correct approach
        debug_printf("Calling android_main...\n");
        android_main();

        debug_printf("Game completed\n");
    } else {
        debug_printf("ERROR: Could not find android_main\n");
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);
        sceKernelDelayThread(5000000);
    }

    cleanup:
    debug_printf("Cleaning up...\n");
    so_cleanup(&fluffydiver_mod);
    vglEnd();

    if (debug_log) {
        debug_printf("Port completed\n");
        fclose(debug_log);
    }

    sceKernelExitProcess(0);
    return 0;
}

// Required wrapper functions
void *__wrap_memcpy(void *dest, const void *src, size_t n) {
    return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n) {
    return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n) {
    return sceClibMemset(s, c, n);
}
