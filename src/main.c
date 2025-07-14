/*
 * main.c - Based on GTA SA Vita entry point methodology
 * Simplified to follow exact GTA SA Vita pattern
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

// Forward declaration of comprehensive analysis function (in so_util.c)
extern int so_analyze_and_try_symbols(so_module *mod, void *fake_env, void *fake_context);

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
    debugPrintf("About to call jni_init()...\n");
    jni_init();
    debugPrintf("jni_init() completed successfully\n");

    debugPrintf("Flushing caches...\n");
    // Flush caches - CRITICAL STEP FROM GTA SA VITA
    debugPrintf("About to flush caches...\n");
    so_flush_caches(&fluffydiver_mod);
    debugPrintf("Cache flush completed\n");

    debugPrintf("Initializing module...\n");
    // Initialize module - CRITICAL STEP FROM GTA SA VITA
    debugPrintf("About to initialize module...\n");
    if (so_initialize(&fluffydiver_mod) < 0) {
        fatal_error("Failed to initialize module");
    }
    debugPrintf("Module initialization completed\n");

    // Show yellow screen before calling game
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("=== GTA SA VITA ENTRY POINT PATTERN ===\n");

    // FIRST: Try JNI_OnLoad - many Android games require this for initialization
    uintptr_t jni_onload_addr = so_symbol(&fluffydiver_mod, "JNI_OnLoad");
    if (jni_onload_addr != 0) {
        debugPrintf("Found JNI_OnLoad at 0x%08X - calling first for initialization\n", jni_onload_addr);

        // JNI_OnLoad signature: jint JNI_OnLoad(JavaVM* vm, void* reserved)
        typedef int (*jni_onload_t)(void* vm, void* reserved);
        jni_onload_t onload_func = (jni_onload_t)jni_onload_addr;

        // Create fake JavaVM
        static void *fake_vm = (void*)0x99999999;

        debugPrintf("Calling JNI_OnLoad...\n");
        int onload_result = onload_func(&fake_vm, NULL);
        debugPrintf("JNI_OnLoad returned: 0x%08X\n", onload_result);
    } else {
        debugPrintf("No JNI_OnLoad found - skipping\n");
    }

    // SECOND: Try main entry point
    uintptr_t entry_addr = so_symbol(&fluffydiver_mod, "Java_com_hotdog_jni_Natives_onHotDogCreate");

    if (entry_addr == 0) {
        debugPrintf("Entry point not found, trying alternatives...\n");

        // Try other common entry points
        const char* alternatives[] = {
            "Java_com_hotdog_jni_Natives_init",
            "Java_com_hotdog_jni_Natives_nativeInit",
            "Java_com_hotdog_jni_Natives_onCreate",
            "android_main",
            "main",
            NULL
        };

        for (int i = 0; alternatives[i] != NULL; i++) {
            entry_addr = so_symbol(&fluffydiver_mod, alternatives[i]);
            if (entry_addr != 0) {
                debugPrintf("Found alternative entry point: %s at 0x%08X\n", alternatives[i], entry_addr);
                break;
            }
        }
    }

    if (entry_addr == 0) {
        debugPrintf("No standard entry points found - trying comprehensive analysis\n");

        // Call comprehensive symbol analysis function
        int analysis_result = so_analyze_and_try_symbols(&fluffydiver_mod, fake_env, fake_context);
        if (analysis_result == 0) {
            debugPrintf("✓ Found working entry point via comprehensive analysis!\n");

            // If we get here, the game is running
            // Cyan screen for complete success
            glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            vglSwapBuffers(GL_FALSE);

            debugPrintf("=== INITIALIZATION COMPLETE ===\n");
            debugPrintf("Game should now be running...\n");

            // Main loop - keep the game alive
            debugPrintf("Entering main loop...\n");
            while (1) {
                // Process input, render, etc.
                sceKernelDelayThread(16666); // ~60 FPS
            }
        } else {
            fatal_error("Comprehensive analysis failed to find working entry point");
        }
    }

    // CRITICAL: Even if we found the standard entry point, it might not work
    // Let's validate it first before trying
    debugPrintf("=== VALIDATING STANDARD ENTRY POINT ===\n");

    // Check if the found entry point has valid code
    uint32_t *code_ptr = (uint32_t*)entry_addr;
    uint32_t first_inst = code_ptr[0];
    uint16_t *thumb_ptr = (uint16_t*)entry_addr;
    uint16_t thumb_inst = thumb_ptr[0];

    debugPrintf("Entry point code analysis:\n");
    debugPrintf("ARM instruction: 0x%08X\n", first_inst);
    debugPrintf("Thumb instruction: 0x%04X\n", thumb_inst);

    // Check for valid ARM or Thumb patterns
    int has_valid_arm = (first_inst & 0xffff0000) == 0xe92d0000;
    int has_valid_thumb = (thumb_inst & 0xFF00) == 0xB500;

    if (!has_valid_arm && !has_valid_thumb) {
        debugPrintf("WARNING: Standard entry point has invalid code patterns!\n");
        debugPrintf("Falling back to comprehensive analysis...\n");

        // Call comprehensive symbol analysis function
        int analysis_result = so_analyze_and_try_symbols(&fluffydiver_mod, fake_env, fake_context);
        if (analysis_result == 0) {
            debugPrintf("✓ Found working entry point via comprehensive analysis!\n");

            // If we get here, the game is running
            // Cyan screen for complete success
            glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            vglSwapBuffers(GL_FALSE);

            debugPrintf("=== INITIALIZATION COMPLETE ===\n");
            debugPrintf("Game should now be running...\n");

            // Main loop - keep the game alive
            debugPrintf("Entering main loop...\n");
            while (1) {
                // Process input, render, etc.
                sceKernelDelayThread(16666); // ~60 FPS
            }
        } else {
            fatal_error("Both standard and comprehensive analysis failed");
        }
    }

    debugPrintf("=== CALLING ENTRY POINT ===\n");
    debugPrintf("Entry point address: 0x%08X\n", entry_addr);

    // Verify address is in module bounds
    uintptr_t base = (uintptr_t)fluffydiver_mod.base;
    if (entry_addr < base || entry_addr >= base + fluffydiver_mod.size) {
        fatal_error("Entry point address outside module bounds");
    }

    debugPrintf("Address verification passed: 0x%08X is within bounds 0x%08X-0x%08X\n",
                entry_addr, base, base + fluffydiver_mod.size);

    // Magenta screen for calling entry point
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    // Flush logs before calling
    if (debug_log) fflush(debug_log);
    if (debug_fd >= 0) sceIoClose(debug_fd);

    debugPrintf("=== DIRECT FUNCTION CALL (GTA SA VITA METHOD) ===\n");

    // Try calling with the most basic approach first
    debugPrintf("Attempting standard JNI function call...\n");

    // Standard JNI call
    typedef void (*jni_func_t)(void* env, void* thiz);
    jni_func_t jni_call = (jni_func_t)entry_addr;

    debugPrintf("About to call with JNI parameters...\n");
    jni_call(fake_env, fake_context);
    debugPrintf("✓ JNI call succeeded!\n");

    // If we get here, the function succeeded
    // Cyan screen for complete success
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

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
