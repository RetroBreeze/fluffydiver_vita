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

// External declarations (pthread_init is already in pthread.h)
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

    // Hook Android logging
    hook_addr(so_symbol(&fluffydiver_mod, "__android_log_print"), (uintptr_t)&ret0);
    hook_addr(so_symbol(&fluffydiver_mod, "__android_log_write"), (uintptr_t)&ret0);
    hook_addr(so_symbol(&fluffydiver_mod, "__android_log_vprint"), (uintptr_t)&ret0);

    // Hook file operations
    hook_addr(so_symbol(&fluffydiver_mod, "fopen"), (uintptr_t)&fopen_hook);
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

    debugPrintf("About to initialize touch and controls...\n");

    debugPrintf("Initializing touch and controls...\n");
    // GTA SA Vita initialization sequence
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

    debugPrintf("Touch initialized, about to call pthread_init...\n");

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

    debugPrintf("Green screen shown, continuing...\n");

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

    debugPrintf("Looking for entry points...\n");
    // Find and call Fluffy Diver entry point
    void (*onHotDogCreate)(void*, void*) = (void*)so_symbol(&fluffydiver_mod,
                                                            "Java_com_hotdog_jni_Natives_onHotDogCreate");

    if (onHotDogCreate) {
        debugPrintf("Found onHotDogCreate at %p\n", onHotDogCreate);
        debugPrintf("fake_env = %p, fake_context = %p\n", fake_env, fake_context);
        debugPrintf("Calling onHotDogCreate...\n");

        // Flush logs before calling
        if (debug_fd >= 0) sceIoClose(debug_fd);
        if (debug_log) fclose(debug_log);

        // Add a small delay before calling
        sceKernelDelayThread(100000); // 0.1 second

        // Show purple screen to indicate we're calling the game
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        // Call the function
        onHotDogCreate(fake_env, fake_context);

        // If we get here, it worked!
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f); // Cyan = success
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        debugPrintf("onHotDogCreate returned!\n");
    } else {
        debugPrintf("onHotDogCreate not found, trying android_main...\n");
        // Try android_main
        void (*android_main)(void*, size_t) = (void*)so_symbol(&fluffydiver_mod, "android_main");
        if (android_main) {
            debugPrintf("Found android_main at %p\n", android_main);
            debugPrintf("Calling android_main...\n");
            android_main(NULL, 0);
        } else {
            debugPrintf("No entry point found!\n");
            fatal_error("No entry point found");
        }
    }

    debugPrintf("Game main loop starting...\n");
    // Main loop
    while (1) {
        sceKernelDelayThread(100000);
    }

    return 0;
}
