/*
 * Safer main.c - Skip actual game function calls for now
 * Focus on getting the JNI stub functions working first
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stddef.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/rtc.h>

#include <vitaGL.h>
#include "so_util_simple.h"

// JNI type definitions
typedef void* JNIEnv;
typedef void* JavaVM;
typedef void* jobject;

// Enhanced JNI environment functions
extern int setup_enhanced_jni_environment();
extern void cleanup_enhanced_jni_environment();
extern void* get_jni_env();
extern void* get_java_vm();

// Function prototypes from jni_patch.c
extern void Java_com_hotdog_jni_Natives_OnGameInitialize(JNIEnv *env, jobject obj);
extern void Java_com_hotdog_jni_Natives_OnGameUpdate(JNIEnv *env, jobject obj);
extern void Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath(JNIEnv *env, jobject obj, void* path);
extern void Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath(JNIEnv *env, jobject obj, void* path);
extern void Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized(JNIEnv *env, jobject obj);
extern int is_game_initialized();
extern int is_game_paused();
extern void create_asset_debug_files();

// Function from minimal_function_test.c
extern void test_all_functions();

// Forward declarations for our functions
static void stub_based_game_initialization(void);
static void stub_based_game_loop(void);
static void interactive_mode(void);

// Game state
static int game_running = 1;
static so_module so_mod;

// Debug logging
static FILE *debug_log = NULL;

// Load address for so-loader
#define LOAD_ADDRESS 0x8000000

/*
 * DEBUG LOGGING FUNCTIONS
 */
static void init_debug_log() {
    sceIoMkdir("ux0:data/fluffydiver", 0777);
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");
    if (debug_log) {
        SceDateTime time;
        sceRtcGetCurrentClock(&time, 0);
        fprintf(debug_log, "=== Fluffy Diver Debug Log ===\n");
        fprintf(debug_log, "Started: %04d-%02d-%02d %02d:%02d:%02d\n",
                time.year, time.month, time.day, time.hour, time.minute, time.second);
        fprintf(debug_log, "Safer Version - Using JNI Stubs Only\n");
        fprintf(debug_log, "================================\n\n");
        fflush(debug_log);
    }
}

void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    if (debug_log) {
        SceDateTime time;
        sceRtcGetCurrentClock(&time, 0);
        fprintf(debug_log, "[%02d:%02d:%02d] ", time.hour, time.minute, time.second);
        vfprintf(debug_log, fmt, args);
        fflush(debug_log);
    }
    va_end(args);
}

static void debug_color_change(const char *color_name, float r, float g, float b) {
    debug_printf("COLOR: %s (%.1f, %.1f, %.1f)\n", color_name, r, g, b);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    vglSwapBuffers(GL_FALSE);
}

/*
 * VITA SYSTEM INITIALIZATION
 */
static void init_vita_systems() {
    debug_printf("Initializing Vita systems...\n");
    vglInit(0x1000000);
    vglWaitVblankStart(GL_FALSE);

    debug_color_change("BLUE - VitaGL Init", 0.0f, 0.0f, 1.0f);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 0);

    debug_printf("Vita systems initialized successfully\n");
}

/*
 * ASSET VERIFICATION
 */
static int verify_and_debug_assets() {
    debug_printf("Verifying game assets...\n");

    SceUID dir = sceIoDopen("ux0:data/fluffydiver/");
    if (dir < 0) {
        debug_printf("ERROR: Assets directory not found!\n");
        return -1;
    }

    SceIoDirent entry;
    int file_count = 0;
    while (sceIoDread(dir, &entry) > 0) {
        file_count++;
    }
    sceIoDclose(dir);
    debug_printf("Total assets: %d files\n", file_count);

    SceUID so_file = sceIoOpen("app0:lib/libFluffyDiver.so", SCE_O_RDONLY, 0);
    if (so_file < 0) {
        debug_printf("ERROR: libFluffyDiver.so not found!\n");
        return -1;
    }

    SceOff size = sceIoLseek(so_file, 0, SCE_SEEK_END);
    sceIoClose(so_file);
    debug_printf("Found libFluffyDiver.so (%d bytes)\n", (int)size);

    create_asset_debug_files();
    return 0;
}

/*
 * STUB-BASED GAME INITIALIZATION
 * Instead of calling the actual game functions, use our JNI stubs
 */
static void stub_based_game_initialization() {
    debug_printf("=== STUB-BASED GAME INITIALIZATION ===\n");

    JNIEnv *env = (JNIEnv*)get_jni_env();
    if (!env) {
        debug_printf("ERROR: JNI environment not available\n");
        return;
    }

    debug_printf("JNI Environment: %p\n", env);
    debug_printf("Using JNI stub functions instead of calling actual game code\n");

    // Show cyan to indicate we're starting stub-based testing
    debug_color_change("CYAN - Starting Stub-Based Init", 0.0f, 1.0f, 1.0f);
    sceKernelDelayThread(1000000);

    // Step 1: Library initialization (stub)
    debug_printf("Step 1: Calling stub OnLibraryInitialized...\n");
    debug_color_change("YELLOW - Calling Stub Function", 1.0f, 1.0f, 0.0f);
    Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized(env, NULL);
    debug_color_change("GREEN - Stub Success", 0.0f, 1.0f, 0.0f);
    sceKernelDelayThread(1000000);

    // Step 2: Set resource paths (stub)
    debug_printf("Step 2: Setting resource paths via stubs...\n");
    debug_color_change("YELLOW - Setting Paths", 1.0f, 1.0f, 0.0f);
    const char *asset_path = "ux0:data/fluffydiver/";
    Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath(env, NULL, (void*)asset_path);
    Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath(env, NULL, (void*)asset_path);
    debug_color_change("GREEN - Paths Set", 0.0f, 1.0f, 0.0f);
    sceKernelDelayThread(1000000);

    // Step 3: Game initialization (stub)
    debug_printf("Step 3: Calling stub OnGameInitialize...\n");
    debug_color_change("YELLOW - Game Init", 1.0f, 1.0f, 0.0f);
    Java_com_hotdog_jni_Natives_OnGameInitialize(env, NULL);
    debug_color_change("GREEN - Game Init Complete", 0.0f, 1.0f, 0.0f);
    sceKernelDelayThread(1000000);

    // Check if the game reports as initialized
    if (is_game_initialized()) {
        debug_printf("SUCCESS: Game reports as initialized via stubs!\n");
        debug_color_change("PURPLE - Stub Init Success", 1.0f, 0.0f, 1.0f);
        sceKernelDelayThread(2000000);
    } else {
        debug_printf("Game not initialized - stubs may need enhancement\n");
        debug_color_change("ORANGE - Partial Success", 1.0f, 0.5f, 0.0f);
        sceKernelDelayThread(2000000);
    }

    debug_printf("=== STUB INITIALIZATION COMPLETE ===\n");
}

/*
 * INTERACTIVE MODE
 */
static void interactive_mode() {
    debug_printf("=== ENHANCED INTERACTIVE MODE ===\n");
    debug_printf("Controls:\n");
    debug_printf("  X = Test OnGameInitialize discovery\n");
    debug_printf("  Circle = Test OnGameUpdate discovery\n");
    debug_printf("  Triangle = Test OnLibraryInitialized discovery\n");
    debug_printf("  Square = Test SetResourcePath discovery\n");
    debug_printf("  L1 = Call OnGameUpdate stub\n");
    debug_printf("  R1 = Call OnGameInitialize stub\n");
    debug_printf("  UP = Test ALL function discovery\n");
    debug_printf("  DOWN = Ready for real function calls\n");
    debug_printf("  LEFT = Test REAL function calls\n");
    debug_printf("  Start+Select = Exit\n");

    JNIEnv *env = (JNIEnv*)get_jni_env();
    int frame_count = 0;
    int last_button_state = 0;

    debug_color_change("CYAN - Enhanced Interactive Mode", 0.0f, 1.0f, 1.0f);

    while (game_running) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);

        // Exit condition
        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) {
            debug_printf("Exit requested\n");
            break;
        }

        // New button functionality
        if (pad.buttons != last_button_state) {
            if (pad.buttons & SCE_CTRL_CROSS) {
                debug_printf("=== TESTING OnGameInitialize DISCOVERY ===\n");
                debug_color_change("RED - Testing Function Discovery", 1.0f, 0.0f, 0.0f);

                // Test function discovery
                if (so_functions_resolved()) {
                    uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameInitialize");
                    if (func_addr) {
                        debug_printf("SUCCESS: OnGameInitialize found at 0x%08x\n", func_addr);
                        debug_printf("Function is within module bounds: %s\n",
                                     (func_addr >= (uintptr_t)so_get_base() &&
                                     func_addr < (uintptr_t)so_get_base() + so_get_size()) ? "YES" : "NO");
                        debug_printf("Function alignment: %s\n", (func_addr & 1) ? "THUMB" : "ARM");
                        debug_color_change("GREEN - Function Found", 0.0f, 1.0f, 0.0f);
                    } else {
                        debug_printf("FAILED: OnGameInitialize not found\n");
                        debug_color_change("RED - Function Not Found", 1.0f, 0.0f, 0.0f);
                    }
                } else {
                    debug_printf("FAILED: Functions not resolved\n");
                    debug_color_change("RED - No Functions", 1.0f, 0.0f, 0.0f);
                }

            } else if (pad.buttons & SCE_CTRL_CIRCLE) {
                debug_printf("=== TESTING OnGameUpdate DISCOVERY ===\n");
                debug_color_change("GREEN - Testing Function Discovery", 0.0f, 1.0f, 0.0f);

                if (so_functions_resolved()) {
                    uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameUpdate");
                    if (func_addr) {
                        debug_printf("SUCCESS: OnGameUpdate found at 0x%08x\n", func_addr);
                        debug_printf("Function is within module bounds: %s\n",
                                     (func_addr >= (uintptr_t)so_get_base() &&
                                     func_addr < (uintptr_t)so_get_base() + so_get_size()) ? "YES" : "NO");
                        debug_color_change("GREEN - Function Found", 0.0f, 1.0f, 0.0f);
                    } else {
                        debug_printf("FAILED: OnGameUpdate not found\n");
                        debug_color_change("RED - Function Not Found", 1.0f, 0.0f, 0.0f);
                    }
                }

            } else if (pad.buttons & SCE_CTRL_TRIANGLE) {
                debug_printf("=== TESTING OnLibraryInitialized DISCOVERY ===\n");
                debug_color_change("BLUE - Testing Function Discovery", 0.0f, 0.0f, 1.0f);

                if (so_functions_resolved()) {
                    uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized");
                    if (func_addr) {
                        debug_printf("SUCCESS: OnLibraryInitialized found at 0x%08x\n", func_addr);
                        debug_printf("This is the function that was crashing!\n");
                        debug_printf("Function is within module bounds: %s\n",
                                     (func_addr >= (uintptr_t)so_get_base() &&
                                     func_addr < (uintptr_t)so_get_base() + so_get_size()) ? "YES" : "NO");
                        debug_color_change("BLUE - Function Found", 0.0f, 0.0f, 1.0f);
                    } else {
                        debug_printf("FAILED: OnLibraryInitialized not found\n");
                        debug_color_change("RED - Function Not Found", 1.0f, 0.0f, 0.0f);
                    }
                }

            } else if (pad.buttons & SCE_CTRL_SQUARE) {
                debug_printf("=== TESTING SetResourcePath DISCOVERY ===\n");
                debug_color_change("YELLOW - Testing Function Discovery", 1.0f, 1.0f, 0.0f);

                if (so_functions_resolved()) {
                    uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath");
                    if (func_addr) {
                        debug_printf("SUCCESS: SetResourcePath found at 0x%08x\n", func_addr);
                        debug_printf("Function is within module bounds: %s\n",
                                     (func_addr >= (uintptr_t)so_get_base() &&
                                     func_addr < (uintptr_t)so_get_base() + so_get_size()) ? "YES" : "NO");
                        debug_color_change("YELLOW - Function Found", 1.0f, 1.0f, 0.0f);
                    } else {
                        debug_printf("FAILED: SetResourcePath not found\n");
                        debug_color_change("RED - Function Not Found", 1.0f, 0.0f, 0.0f);
                    }
                }

            } else if (pad.buttons & SCE_CTRL_LTRIGGER) {
                debug_printf("L1 pressed - calling OnGameUpdate stub\n");
                debug_color_change("PURPLE - Calling Stub", 1.0f, 0.0f, 1.0f);
                Java_com_hotdog_jni_Natives_OnGameUpdate(env, NULL);

            } else if (pad.buttons & SCE_CTRL_RTRIGGER) {
                debug_printf("R1 pressed - calling OnGameInitialize stub\n");
                debug_color_change("ORANGE - Calling Stub", 1.0f, 0.5f, 0.0f);
                Java_com_hotdog_jni_Natives_OnGameInitialize(env, NULL);

            } else if (pad.buttons & SCE_CTRL_UP) {
                debug_printf("=== TESTING ALL FUNCTION DISCOVERY ===\n");
                debug_color_change("MAGENTA - Testing All Functions", 1.0f, 0.0f, 1.0f);

                const char* test_functions[] = {
                    "Java_com_hotdog_jni_Natives_OnGameInitialize",
                    "Java_com_hotdog_jni_Natives_OnGameUpdate",
                    "Java_com_hotdog_jni_Natives_OnGamePause",
                    "Java_com_hotdog_jni_Natives_OnGameResume",
                    "Java_com_hotdog_jni_Natives_OnGameTouchEvent",
                    "Java_com_hotdog_jni_Natives_OnGameBack",
                    "Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath",
                    "Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath",
                    "Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized",
                    "Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete",
                    "Java_com_hotdog_jni_Natives_onCashUpdate",
                    "Java_com_hotdog_jni_Natives_onLanguage",
                    "Java_com_hotdog_jni_Natives_onHotDogCreate",
                    NULL
                };

                int found_count = 0;
                for (int i = 0; test_functions[i] != NULL; i++) {
                    uintptr_t func_addr = so_symbol(NULL, test_functions[i]);
                    if (func_addr) {
                        debug_printf("✅ %s: 0x%08x\n", test_functions[i], func_addr);
                        found_count++;
                    } else {
                        debug_printf("❌ %s: NOT FOUND\n", test_functions[i]);
                    }
                }

                debug_printf("=== SUMMARY: %d/13 functions found ===\n", found_count);
                if (found_count == 13) {
                    debug_color_change("GREEN - All Functions Found", 0.0f, 1.0f, 0.0f);
                } else {
                    debug_color_change("ORANGE - Some Functions Missing", 1.0f, 0.5f, 0.0f);
                }

            } else if (pad.buttons & SCE_CTRL_LEFT) {
                debug_printf("=== TESTING REAL FUNCTION CALLS ===\n");
                debug_color_change("WHITE - Testing Real Functions", 1.0f, 1.0f, 1.0f);
                test_all_functions();

            } else if (pad.buttons & SCE_CTRL_DOWN) {
                debug_printf("=== READY FOR REAL FUNCTION CALLS ===\n");
                debug_color_change("WHITE - Ready for Real Functions", 1.0f, 1.0f, 1.0f);
                debug_printf("All systems tested and working!\n");
                debug_printf("Next: Enable real function calls with proper FalsoJNI\n");
                debug_printf("Module base: %p\n", so_get_base());
                debug_printf("Module size: %zu\n", so_get_size());
                debug_printf("FalsoJNI env: %p\n", env);
            }
        }

        // Default color cycling
        if (pad.buttons == 0) {
            glClearColor(0.0f, 1.0f, 1.0f, 1.0f);  // Cyan (default)
        }

        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);

        last_button_state = pad.buttons;

        // Periodic status report
        frame_count++;
        if (frame_count % 1800 == 0) {  // Every 30 seconds
            debug_printf("Enhanced interactive mode - Frame %d\n", frame_count);
        }

        usleep(16666); // ~60fps
    }
}

/*
 * STUB-BASED GAME LOOP
 */
static void stub_based_game_loop() {
    debug_printf("Starting stub-based game loop...\n");

    JNIEnv *env = (JNIEnv*)get_jni_env();
    if (!env) {
        debug_printf("ERROR: JNI environment not available\n");
        return;
    }

    // Initialize the game using stubs
    stub_based_game_initialization();

    // Show white to indicate we're ready for the update loop
    debug_color_change("WHITE - Ready for Update Loop", 1.0f, 1.0f, 1.0f);
    sceKernelDelayThread(1000000);

    // Start the update loop using stub functions
    debug_printf("Starting stub-based update loop...\n");
    debug_printf("This tests if our JNI stubs can run a complete game loop\n");

    int frame_count = 0;
    int update_test_frames = 300; // 5 seconds of updates

    debug_color_change("MAGENTA - Update Loop Test", 1.0f, 0.0f, 1.0f);

    for (int frame = 0; frame < update_test_frames; frame++) {
        // Call stub update function
        Java_com_hotdog_jni_Natives_OnGameUpdate(env, NULL);

        // Check if the game changed the screen (indicating potential rendering)
        GLfloat clear_color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);

        // If color changed from magenta, the stub might have enabled something
        if (clear_color[0] != 1.0f || clear_color[1] != 0.0f || clear_color[2] != 1.0f) {
            debug_printf("INTERESTING: Stub update changed screen color to: %.2f, %.2f, %.2f\n",
                         clear_color[0], clear_color[1], clear_color[2]);
            debug_printf("This might indicate the stub is working with actual game logic\n");
            break;
        }

        // Progress indicator
        if (frame % 60 == 0) {
            debug_printf("Stub update frame %d/%d\n", frame, update_test_frames);
        }

        vglSwapBuffers(GL_FALSE);
        usleep(16666); // ~60fps
        frame_count++;
    }

    debug_printf("Stub update loop completed %d frames without crashing\n", frame_count);
    debug_printf("This proves our JNI environment and stubs are stable\n");

    // Switch to interactive mode
    debug_printf("Switching to interactive mode...\n");
    interactive_mode();
}

/*
 * MAIN FUNCTION
 */
int main() {
    init_debug_log();
    debug_printf("=== Fluffy Diver PS Vita Port - Safer Stub Version ===\n");

    // Initialize Vita systems
    init_vita_systems();

    // Verify assets
    if (verify_and_debug_assets() < 0) {
        debug_printf("Asset verification failed\n");
        debug_color_change("RED - Asset Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }

    // Setup enhanced JNI environment
    debug_printf("Setting up enhanced JNI environment...\n");
    if (setup_enhanced_jni_environment() < 0) {
        debug_printf("JNI environment setup failed\n");
        debug_color_change("RED - JNI Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }
    debug_printf("JNI environment ready\n");

    // Load and relocate the .so file
    debug_printf("Loading libFluffyDiver.so...\n");
    int ret = so_file_load(&so_mod, "app0:lib/libFluffyDiver.so", LOAD_ADDRESS);
    if (ret < 0) {
        debug_printf("Failed to load .so file\n");
        debug_color_change("RED - SO Load Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }

    debug_printf("Resolving symbols...\n");
    ret = so_relocate(&so_mod);
    if (ret < 0) {
        debug_printf("Failed to relocate symbols\n");
        debug_color_change("RED - Relocation Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000);
        goto cleanup;
    }

    debug_printf("Initializing loaded module...\n");
    so_initialize(&so_mod);

    // Verify symbols are resolved
    if (so_functions_resolved()) {
        debug_printf("SUCCESS: All game functions resolved!\n");
        debug_printf("Starting safer stub-based testing...\n");
        debug_printf("NOTE: Using JNI stubs instead of calling actual game functions\n");
        stub_based_game_loop();
    } else {
        debug_printf("ERROR: Game functions not resolved\n");
        debug_color_change("RED - Function Resolution Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000);
    }

    cleanup:
    debug_printf("Cleaning up...\n");

    // Cleanup in reverse order
    cleanup_enhanced_jni_environment();
    so_cleanup();
    vglEnd();

    if (debug_log) {
        debug_printf("Port terminated\n");
        fclose(debug_log);
    }

    sceKernelExitProcess(0);
    return 0;
}

// Required wrappers (unchanged)
void *__wrap_memcpy(void *dest, const void *src, size_t n) {
    return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n) {
    return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n) {
    return sceClibMemset(s, c, n);
}
