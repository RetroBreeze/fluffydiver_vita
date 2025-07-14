/*
 * main.c - Simple GTA SA Vita methodology with direct function call
 * Based on your original working approach + actual entry point
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

int main(int argc, char *argv[]) {
    // Create debug directory and open log FIRST
    sceIoMkdir("ux0:data/", 0777);
    sceIoMkdir("ux0:data/fluffydiver/", 0777);

    // Open debug log with direct I/O for immediate writes
    debug_fd = sceIoOpen("ux0:data/fluffydiver/debug_simple.log", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");

    debugPrintf("=== Fluffy Diver PS Vita Port - Simple Call ===\n");
    debugPrintf("Based on GTA SA Vita by TheOfficialFloW\n");
    debugPrintf("Direct call to discovered entry point\n\n");

    // Initialize SceAppUtil (from GTA SA Vita)
    debugPrintf("Initializing SceAppUtil...\n");
    SceAppUtilInitParam init_param;
    SceAppUtilBootParam boot_param;
    memset(&init_param, 0, sizeof(SceAppUtilInitParam));
    memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&init_param, &boot_param);

    debugPrintf("Initializing touch and controls...\n");
    // GTA SA Vita initialization sequence
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

    debugPrintf("Initializing pthread...\n");
    int pthread_ret = pthread_init();
    debugPrintf("pthread_init returned: %d\n", pthread_ret);

    debugPrintf("Creating data directory...\n");
    sceIoMkdir("ux0:data/fluffydiver", 0777);

    debugPrintf("Initializing VitaGL...\n");
    vglInitExtended(0, 960, 544, 0x1800000, SCE_GXM_MULTISAMPLE_4X);
    vglUseVram(GL_TRUE);

    // Green screen - initialization
    debugPrintf("VitaGL initialized, showing green screen...\n");
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("Loading libFluffyDiver.so...\n");
    if (so_load(&fluffydiver_mod, "app0:lib/libFluffyDiver.so", LOAD_ADDRESS) < 0) {
        fatal_error("Failed to load shared object");
    }
    debugPrintf("SO loaded successfully at 0x%08X\n", LOAD_ADDRESS);

    debugPrintf("Relocating...\n");
    if (so_relocate(&fluffydiver_mod) < 0) {
        fatal_error("Failed to relocate");
    }

    debugPrintf("Resolving symbols...\n");
    if (so_resolve(&fluffydiver_mod, default_dynlib, default_dynlib_size, 0) < 0) {
        fatal_error("Failed to resolve symbols");
    }

    debugPrintf("Patching game...\n");
    patch_game();

    debugPrintf("Setting up JNI...\n");
    jni_init();

    debugPrintf("Flushing caches...\n");
    so_flush_caches(&fluffydiver_mod);

    debugPrintf("Initializing module...\n");
    if (so_initialize(&fluffydiver_mod) < 0) {
        fatal_error("Failed to initialize module");
    }

    // Yellow screen - about to call game
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("=== CALLING DISCOVERED ENTRY POINT ===\n");

    // Look for the entry point we found
    uintptr_t entry_addr = so_symbol(&fluffydiver_mod, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized");

    if (entry_addr == 0) {
        debugPrintf("ERROR: OnLibraryInitialized entry point not found\n");
        fatal_error("Entry point not found");
    }

    debugPrintf("Found OnLibraryInitialized at 0x%08X\n", entry_addr);

    // Validate ARM code
    uint32_t *code_ptr = (uint32_t*)entry_addr;
    uint32_t first_inst = code_ptr[0];
    debugPrintf("First instruction: 0x%08X\n", first_inst);

    if ((first_inst & 0xffff0000) != 0xe92d0000) {
        debugPrintf("WARNING: Unexpected instruction pattern\n");
    } else {
        debugPrintf("✓ Valid ARM prologue detected\n");
    }

    // Start hang detection
    function_returned = 0;
    SceUID hang_thread = sceKernelCreateThread("hang_detect", hang_detection_thread, 0x10000100, 0x10000, 0, 0, NULL);

    if (hang_thread >= 0) {
        sceKernelStartThread(hang_thread, 0, NULL);
        debugPrintf("Started hang detection thread\n");
    }

    // Blue screen - about to call
    glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("About to call OnLibraryInitialized...\n");
    debugPrintf("Parameters: fake_env=%p, fake_context=%p\n", fake_env, fake_context);

    // THE CALL - with GTA SA Vita style error handling
    typedef void (*OnLibraryInitialized_t)(void* env, void* thiz);
    OnLibraryInitialized_t init_func = (OnLibraryInitialized_t)entry_addr;

    debugPrintf("Calling game initialization...\n");
    debugPrintf("Entry point address: 0x%08X\n", entry_addr);
    debugPrintf("Function pointer: %p\n", init_func);

    // Flush logs before call
    if (debug_log) fflush(debug_log);
    if (debug_fd >= 0) sceIoSync(debug_fd, 0);

    debugPrintf("=== CRITICAL CALL STARTING ===\n");

    // GTA SA Vita approach: Try with NULL parameters first (safer)
    debugPrintf("Attempting call with NULL parameters...\n");
    init_func(NULL, NULL);
    debugPrintf("=== CRITICAL CALL COMPLETED ===\n");

    // If we get here, it returned
    function_returned = 1;
    debugPrintf("✓ Function returned successfully!\n");

    // Green screen - success
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);

    debugPrintf("=== GAME INITIALIZATION COMPLETE ===\n");

    // Simple wait loop
    debugPrintf("Press START+SELECT to exit...\n");
    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);

        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) {
            break;
        }

        sceKernelDelayThread(16666);
    }

    // Cleanup
    if (debug_log) fclose(debug_log);
    if (debug_fd >= 0) sceIoClose(debug_fd);

    return 0;
}
