/*
 * Fluffy Diver PS Vita - Based on GTA SA Vita by TheOfficialFloW
 */

#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>
#include <psp2/apputil.h>
#include <psp2/appmgr.h>
#include <kubridge.h>
#include <vitaGL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

#include "so_util.h"
#include "jni_patch.h"

// External declarations
extern FILE *fopen_hook(const char *filename, const char *mode);
extern DynLibFunction default_dynlib[];
extern size_t default_dynlib_size;

// Memory settings from GTA SA Vita
int _newlib_heap_size_user = 240 * 1024 * 1024;
unsigned int _pthread_stack_default_user = 1 * 1024 * 1024;
unsigned int sceLibcHeapSize = 240 * 1024 * 1024;

// Module
so_module fluffydiver_mod;

// Load address - same as GTA SA Vita
#define LOAD_ADDRESS 0x98000000

// Debug log file
static FILE *debug_log = NULL;
static SceUID debug_fd = -1;

void debugPrintf(const char *fmt, ...) {
    va_list args;
    char buffer[512];

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Print to console
    printf("%s", buffer);

    // Write directly using sceIoWrite for immediate output
    if (debug_fd >= 0) {
        sceIoWrite(debug_fd, buffer, strlen(buffer));
    }

    // Also write to FILE* for compatibility
    if (debug_log) {
        fprintf(debug_log, "%s", buffer);
        fflush(debug_log);
    }
}

void fatal_error(const char *fmt, ...) {
    va_list list;
    char msg[256];
    va_start(list, fmt);
    vsnprintf(msg, sizeof(msg), fmt, list);
    va_end(list);

    debugPrintf("FATAL: %s\n", msg);

    // Red screen of death
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    sceKernelDelayThread(5 * 1000 * 1000);
    sceKernelExitProcess(0);
}

void patch_game(void) {
    debugPrintf("Patching game for Fluffy Diver...\n");
    // GTA SA Vita approach: Most patching is done through symbol resolution
    debugPrintf("Game patching complete\n");
}

int main(int argc, char *argv[]) {
    // Create debug directory and open log FIRST
    sceIoMkdir("ux0:data/", 0777);
    sceIoMkdir("ux0:data/fluffydiver/", 0777);

    // Open debug log with direct I/O for immediate writes
    debug_fd = sceIoOpen("ux0:data/fluffydiver/debug_direct.log", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");

    debugPrintf("=== Fluffy Diver PS Vita Port ===\n");
    debugPrintf("Based on GTA SA Vita by TheOfficialFloW\n\n");
    debugPrintf("Debug log opened successfully\n");
    debugPrintf("Heap size: %d MB\n", _newlib_heap_size_user / (1024*1024));
    debugPrintf("Thread stack: %d MB\n", _pthread_stack_default_user / (1024*1024));

    // Initialize SceAppUtil (from GTA SA Vita)
    debugPrintf("Initializing SceAppUtil...\n");
    SceAppUtilInitParam init_param;
    SceAppUtilBootParam boot_param;
    memset(&init_param, 0, sizeof(SceAppUtilInitParam));
    memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&init_param, &boot_param);
    debugPrintf("SceAppUtil initialized\n");

    debugPrintf("Initializing touch and controls...\n");
    // GTA SA Vita initialization sequence
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

    debugPrintf("Initializing pthread...\n");
    // Setup memory and threading - pthread_init returns int
    int pthread_ret = pthread_init();
    debugPrintf("pthread_init returned: %d\n", pthread_ret);

    debugPrintf("Creating data directory...\n");
    // Create data directory
    sceIoMkdir("ux0:data/fluffydiver", 0777);

    debugPrintf("Initializing VitaGL...\n");
    // Initialize VitaGL
    vglInitExtended(0, 960, 544, 0x1800000, SCE_GXM_MULTISAMPLE_4X);
    vglUseVram(GL_TRUE);

    // NOW we can use OpenGL
    debugPrintf("VitaGL initialized, showing green screen...\n");
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("Loading libFluffyDiver.so...\n");
    // Load shared object
    if (so_load(&fluffydiver_mod, "app0:lib/libFluffyDiver.so", LOAD_ADDRESS) < 0) {
        debugPrintf("ERROR: Failed to load .so file\n");
        fatal_error("Failed to load shared object");
    }
    debugPrintf("SO loaded successfully at 0x%08X\n", LOAD_ADDRESS);

    debugPrintf("Relocating...\n");
    // Relocate
    if (so_relocate(&fluffydiver_mod) < 0) {
        fatal_error("Failed to relocate");
    }
    debugPrintf("Relocation complete\n");

    debugPrintf("Resolving symbols...\n");
    // Resolve symbols - CRITICAL STEP FROM GTA SA VITA
    // Use 0 for non-strict mode (don't fail on missing symbols)
    if (so_resolve(&fluffydiver_mod, default_dynlib, default_dynlib_size, 0) < 0) {
        fatal_error("Failed to resolve symbols");
    }
    debugPrintf("Symbol resolution complete\n");

    debugPrintf("Patching game...\n");
    // Patch game
    patch_game();

    debugPrintf("Setting up JNI...\n");
    // Setup JNI
    jni_init();

    debugPrintf("Flushing caches...\n");
    // Flush caches - CRITICAL STEP FROM GTA SA VITA
    so_flush_caches(&fluffydiver_mod);

    debugPrintf("Initializing module...\n");
    // Initialize module - CRITICAL STEP FROM GTA SA VITA
    if (so_initialize(&fluffydiver_mod) < 0) {
        fatal_error("Failed to initialize module");
    }
    debugPrintf("Module initialized\n");

    // Show yellow screen before calling game
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("=== ENTRY POINT DISCOVERY ===\n");
    debugPrintf("Following GTA SA Vita methodology to find correct entry point\n");

    // GTA SA Vita approach: Scan for all possible entry points first
    const char* entry_points[] = {
        "JNI_OnLoad",                                    // Standard Android JNI initialization
        "Java_com_hotdog_jni_Natives_init",              // Game-specific init
        "Java_com_hotdog_jni_Natives_nativeInit",        // Alternative init
        "Java_com_hotdog_jni_Natives_onCreate",          // Activity lifecycle
        "Java_com_hotdog_jni_Natives_onHotDogCreate",    // Game-specific create
        "Java_com_hotdog_jni_Natives_onStart",           // Activity start
        "Java_com_hotdog_jni_Natives_onResume",          // Activity resume
        "android_main",                                   // Native activity main
        "main",                                           // Standard main
        NULL
    };

    debugPrintf("Scanning symbol table for available entry points:\n");
    int found_entry_points = 0;
    uintptr_t jni_onload_addr = 0;
    uintptr_t primary_entry_addr = 0;
    const char* primary_entry_name = NULL;

    for (int i = 0; entry_points[i] != NULL; i++) {
        uintptr_t addr = so_symbol(&fluffydiver_mod, entry_points[i]);
        if (addr != 0) {
            debugPrintf("  ✓ Found: %s at 0x%08X\n", entry_points[i], addr);
            found_entry_points++;

            // Remember JNI_OnLoad for later
            if (strcmp(entry_points[i], "JNI_OnLoad") == 0) {
                jni_onload_addr = addr;
            }

            // Remember first game-specific entry point
            if (!primary_entry_addr && strstr(entry_points[i], "Java_com_hotdog")) {
                primary_entry_addr = addr;
                primary_entry_name = entry_points[i];
            }

            // If no game-specific entry found, try android_main
            if (!primary_entry_addr && strcmp(entry_points[i], "android_main") == 0) {
                primary_entry_addr = addr;
                primary_entry_name = entry_points[i];
            }

            // Last resort: main
            if (!primary_entry_addr && strcmp(entry_points[i], "main") == 0) {
                primary_entry_addr = addr;
                primary_entry_name = entry_points[i];
            }
        } else {
            debugPrintf("  ✗ Not found: %s\n", entry_points[i]);
        }
    }

    debugPrintf("Found %d entry points total\n", found_entry_points);

    if (found_entry_points == 0) {
        fatal_error("No entry points found in library");
    }

    // GTA SA Vita pattern: Call JNI_OnLoad first if available
    if (jni_onload_addr != 0) {
        debugPrintf("=== CALLING JNI_OnLoad ===\n");
        debugPrintf("Standard Android pattern: Initialize JNI environment first\n");

        // JNI_OnLoad signature: jint JNI_OnLoad(JavaVM* vm, void* reserved)
        int (*jni_onload)(void*, void*) = (int(*)(void*, void*))jni_onload_addr;

        // Create fake JavaVM structure (GTA SA Vita approach)
        static struct {
            void* functions;
            void* reserved[3];
        } fake_javavm = { NULL, {NULL, NULL, NULL} };

        debugPrintf("Calling JNI_OnLoad(JavaVM=%p, reserved=NULL)\n", &fake_javavm);

        // Purple screen for JNI_OnLoad
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        // Flush logs before calling
        if (debug_log) fflush(debug_log);
        if (debug_fd >= 0) sceIoClose(debug_fd);

        int result = jni_onload(&fake_javavm, NULL);

        // Reopen debug log
        debug_fd = sceIoOpen("ux0:data/fluffydiver/debug_direct.log", SCE_O_WRONLY | SCE_O_APPEND, 0777);

        debugPrintf("JNI_OnLoad returned: %d\n", result);

        if (result > 0) {
            debugPrintf("✓ JNI_OnLoad succeeded!\n");
            // Green screen for JNI_OnLoad success
            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            vglSwapBuffers(GL_FALSE);
        } else {
            debugPrintf("⚠ JNI_OnLoad returned error, but continuing...\n");
            // Yellow screen for warning
            glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            vglSwapBuffers(GL_FALSE);
        }

        sceKernelDelayThread(500000); // 0.5 second delay to see result
    }

    // Now call the primary entry point
    if (primary_entry_addr != 0 && primary_entry_name != NULL) {
        debugPrintf("=== CALLING PRIMARY ENTRY POINT ===\n");
        debugPrintf("Entry point: %s at 0x%08X\n", primary_entry_name, primary_entry_addr);

        // Verify address is in module bounds
        uintptr_t base = (uintptr_t)fluffydiver_mod.base;
        if (primary_entry_addr < base || primary_entry_addr >= base + fluffydiver_mod.size) {
            fatal_error("Entry point address outside module bounds");
        }

        // Magenta screen for primary entry point
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        // Flush logs before calling
        if (debug_log) fflush(debug_log);
        if (debug_fd >= 0) sceIoClose(debug_fd);

        // Call based on entry point type
        if (strstr(primary_entry_name, "Java_com_hotdog")) {
            debugPrintf("Calling JNI function: %s(JNIEnv*, jobject)\n", primary_entry_name);
            void (*jni_func)(void*, void*) = (void(*)(void*, void*))primary_entry_addr;
            jni_func(fake_env, fake_context);

        } else if (strcmp(primary_entry_name, "android_main") == 0) {
            debugPrintf("Calling android_main(struct android_app*)\n");
            void (*android_main_func)(void*) = (void(*)(void*))primary_entry_addr;
            android_main_func(fake_context);

        } else if (strcmp(primary_entry_name, "main") == 0) {
            debugPrintf("Calling main(int, char**)\n");
            int (*main_func)(int, char**) = (int(*)(int, char**))primary_entry_addr;
            char *argv[] = {"fluffydiver", NULL};
            main_func(1, argv);
        }

        // Reopen debug log
        debug_fd = sceIoOpen("ux0:data/fluffydiver/debug_direct.log", SCE_O_WRONLY | SCE_O_APPEND, 0777);

        debugPrintf("✓ %s returned successfully!\n", primary_entry_name);

        // Cyan screen for complete success
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

    } else {
        fatal_error("No suitable primary entry point found");
    }

    debugPrintf("=== INITIALIZATION COMPLETE ===\n");
    debugPrintf("Game should now be running...\n");

    // Main loop - keep the game alive
    debugPrintf("Entering main loop...\n");
    while (1) {
        // Process input, render, etc.
        sceKernelDelayThread(16666); // ~60 FPS
    }

    return 0;
}
