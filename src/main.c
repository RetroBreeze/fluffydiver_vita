/*
 * main.c - Restored GTA SA Vita simple methodology with hang detection
 * Based on your original working approach
 */

#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
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

// GTA SA Vita game state detection
static volatile int game_running = 0;
static volatile int function_returned = 0;

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

// GTA SA Vita hang detection thread
int hang_detection_thread(SceSize args, void *argp) {
    debugPrintf("[HANG] Hang detection thread started\n");

    // Wait 5 seconds for function to return
    sceKernelDelayThread(5 * 1000 * 1000);

    if (!function_returned) {
        debugPrintf("[HANG] Function hasn't returned after 5 seconds\n");
        debugPrintf("[HANG] This could mean:\n");
        debugPrintf("[HANG] 1. SUCCESS - Game is running in main loop\n");
        debugPrintf("[HANG] 2. HANG - Function is waiting for something\n");
        debugPrintf("[HANG] 3. CRASH - Function crashed without error\n");

        // Test if system is responsive
        debugPrintf("[HANG] Testing system responsiveness...\n");

        // Try to change screen color to test if we can still render
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f); // Cyan for "possibly working"
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        debugPrintf("[HANG] If screen changed to cyan, system is responsive\n");
        debugPrintf("[HANG] Monitoring for game activity...\n");

        // Monitor for 30 more seconds
        for (int i = 0; i < 30; i++) {
            sceKernelDelayThread(1000 * 1000); // 1 second

            // Check if function ever returns
            if (function_returned) {
                debugPrintf("[HANG] Function returned after %d seconds total\n", 5 + i);
                game_running = 1;
                return 0;
            }

            // Test input responsiveness every 5 seconds
            if (i % 5 == 0) {
                SceCtrlData pad;
                sceCtrlPeekBufferPositive(0, &pad, 1);

                if (pad.buttons != 0) {
                    debugPrintf("[HANG] Input detected: 0x%08X - Game may be responsive!\n", pad.buttons);
                    game_running = 1;
                }
            }
        }

        if (!function_returned) {
            debugPrintf("[HANG] Function still hasn't returned after 35 seconds\n");
            debugPrintf("[HANG] Assuming game is running in main loop (GTA SA Vita pattern)\n");
            game_running = 1;
        }
    }

    return 0;
}

// Forward declaration - function is in so_util.c where ELF structures are defined
extern int so_find_real_entry_points(so_module *mod, void *fake_env, void *fake_context);

int main(int argc, char *argv[]) {
    // Create debug directory and open log FIRST
    sceIoMkdir("ux0:data/", 0777);
    sceIoMkdir("ux0:data/fluffydiver/", 0777);

    // Open debug log with direct I/O for immediate writes
    debug_fd = sceIoOpen("ux0:data/fluffydiver/debug_restored.log", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");

    debugPrintf("=== Fluffy Diver PS Vita Port - Restored ===\n");
    debugPrintf("Based on GTA SA Vita by TheOfficialFloW\n");
    debugPrintf("Restored to your original working approach\n\n");
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

    // Show yellow screen before entry point attempt
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("=== RESTORED GTA SA VITA ENTRY POINT PATTERN ===\n");

    // FIRST: Try JNI_OnLoad
    uintptr_t jni_onload_addr = so_symbol(&fluffydiver_mod, "JNI_OnLoad");
    if (jni_onload_addr != 0) {
        debugPrintf("Found JNI_OnLoad at 0x%08X - calling first\n", jni_onload_addr);

        typedef int (*jni_onload_t)(void* vm, void* reserved);
        jni_onload_t onload_func = (jni_onload_t)jni_onload_addr;

        static void *fake_vm = (void*)0x99999999;

        debugPrintf("Calling JNI_OnLoad...\n");
        int onload_result = onload_func(&fake_vm, NULL);
        debugPrintf("JNI_OnLoad returned: 0x%08X\n", onload_result);
    } else {
        debugPrintf("No JNI_OnLoad found - skipping\n");
    }

    // SECOND: Try standard entry point with validation
    uintptr_t entry_addr = so_symbol(&fluffydiver_mod, "Java_com_hotdog_jni_Natives_onHotDogCreate");

    if (entry_addr != 0) {
        uint32_t *code_ptr = (uint32_t*)entry_addr;
        uint32_t first_inst = code_ptr[0];
        uint16_t *thumb_ptr = (uint16_t*)entry_addr;
        uint16_t thumb_inst = thumb_ptr[0];

        debugPrintf("=== VALIDATING STANDARD ENTRY POINT ===\n");
        debugPrintf("Entry point code analysis:\n");
        debugPrintf("ARM instruction: 0x%08X\n", first_inst);
        debugPrintf("Thumb instruction: 0x%04X\n", thumb_inst);

        // Check for valid ARM or Thumb patterns
        int has_valid_arm = (first_inst & 0xffff0000) == 0xe92d0000;
        int has_valid_thumb = (thumb_inst & 0xFF00) == 0xB500;

        if (has_valid_arm || has_valid_thumb) {
            debugPrintf("✓ Standard entry point has valid code - trying direct call\n");

            // Start hang detection
            function_returned = 0;
            SceUID thid = sceKernelCreateThread("hang_detect", hang_detection_thread, 0x10000100, 0x10000, 0, 0, NULL);
            if (thid >= 0) {
                sceKernelStartThread(thid, 0, NULL);
            }

            typedef void (*jni_func_t)(void* env, void* thiz);
            jni_func_t jni_call = (jni_func_t)entry_addr;

            debugPrintf("About to call standard entry point...\n");
            jni_call(fake_env, fake_context);

            function_returned = 1;
            debugPrintf("✓ Standard entry point returned!\n");
        } else {
            debugPrintf("WARNING: Standard entry point has invalid code patterns!\n");
            debugPrintf("Using smart entry point scanner...\n");

            // Use smart entry point scanner instead of random C++ functions
            int scan_result = so_find_real_entry_points(&fluffydiver_mod, fake_env, fake_context);
            if (scan_result == 0) {
                debugPrintf("✓ Found potential entry points!\n");
            } else {
                debugPrintf("⚠ No suitable entry points found\n");
            }
        }
    } else {
        debugPrintf("No standard entry point found - using smart entry point scanner\n");

        // Use smart entry point scanner
        int scan_result = so_find_real_entry_points(&fluffydiver_mod, fake_env, fake_context);
        if (scan_result == 0) {
            debugPrintf("✓ Found potential entry points!\n");
        } else {
            debugPrintf("⚠ No suitable entry points found\n");
        }
    }

    // The symbol scanner will show us what entry points exist
    // For now, just show the scan results and exit cleanly
    debugPrintf("=== ENTRY POINT ANALYSIS COMPLETE ===\n");
    debugPrintf("Check the log above to see what entry points were found\n");
    debugPrintf("Next step: Manually try calling the most promising entry points\n");

    // Simple wait loop to keep system responsive
    debugPrintf("Press START+SELECT to exit...\n");
    int wait_count = 0;
    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);

        // Exit on Start+Select
        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) {
            debugPrintf("Exit requested by user\n");
            break;
        }

        wait_count++;
        if (wait_count % 300 == 0) { // Every 5 seconds
            debugPrintf("Waiting for exit command... (%d seconds)\n", wait_count / 60);
        }

        sceKernelDelayThread(16666); // ~60 FPS
    }

    // Cleanup
    debugPrintf("=== CLEANUP ===\n");
    if (debug_log) fclose(debug_log);
    if (debug_fd >= 0) sceIoClose(debug_fd);

    return 0;
}
