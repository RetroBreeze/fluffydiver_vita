/*
 * SAFE JNI Patch - NO real function calls until we fix FalsoJNI
 * This will let us test the rest of the system without crashes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include "so_util_simple.h"

// JNI type definitions
typedef void* JNIEnv;
typedef void* JavaVM;
typedef void* jobject;
typedef void* jstring;

// Function calling convention
#define JNIEXPORT
#define JNICALL

// Forward declaration
void Java_com_hotdog_jni_Natives_OnGameTouchEvent(JNIEnv *env, jobject obj, int x, int y, int action);

// Game state
static int game_initialized = 0;
static int game_paused = 0;
static int menu_phase = 0;
static int game_started = 0;

// FalsoJNI environment access
extern JNIEnv* falsojni_get_env();
extern JavaVM* falsojni_get_vm();

/*
 * SAFE JNI FUNCTION IMPLEMENTATIONS
 * These will NOT call real game functions - pure stubs for now
 */

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameInitialize(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] OnGameInitialize called - STUB MODE\n");
    printf("[SAFE-JNI] env=%p, obj=%p\n", env, obj);

    // Get FalsoJNI environment (but don't use it to call real functions)
    JNIEnv *falsojni_env = falsojni_get_env();
    printf("[SAFE-JNI] FalsoJNI environment available at: %p\n", falsojni_env);

    // Check if we COULD call real functions (but don't actually do it)
    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameInitialize");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnGameInitialize at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        } else {
            printf("[SAFE-JNI] Could not find OnGameInitialize function\n");
        }
    } else {
        printf("[SAFE-JNI] Game functions not resolved\n");
    }

    // Set our local state
    game_initialized = 1;
    menu_phase = 1;

    printf("[SAFE-JNI] OnGameInitialize completed successfully (stub mode)\n");
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameUpdate(JNIEnv *env, jobject obj) {
    static int call_count = 0;
    call_count++;

    // Get FalsoJNI environment
    JNIEnv *falsojni_env = falsojni_get_env();

    // Check if we could call real functions (but don't)
    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameUpdate");
        if (func_addr) {
            // Only log occasionally to avoid spam
            if (call_count % 300 == 0) {
                printf("[SAFE-JNI] FOUND real OnGameUpdate at: 0x%08x (call #%d)\n", func_addr, call_count);
                printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
            }
        }
    }

    // Keep auto-progression logic
    static int frame_count = 0;
    frame_count++;

    if (frame_count % 180 == 0) {
        switch (menu_phase) {
            case 1:  // Title screen
                printf("[SAFE-JNI] Auto-progression: Title -> Game (frame %d)\n", frame_count);
                Java_com_hotdog_jni_Natives_OnGameTouchEvent(falsojni_env, obj, 480, 400, 1);
                menu_phase = 2;
                break;
            case 2:  // Game screen
                if (!game_started) {
                    printf("[SAFE-JNI] Auto-progression: Starting dive (frame %d)\n", frame_count);
                    Java_com_hotdog_jni_Natives_OnGameTouchEvent(falsojni_env, obj, 480, 272, 1);
                    game_started = 1;
                }
                break;
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGamePause(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] OnGamePause called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGamePause");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnGamePause at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }

    game_paused = 1;
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameResume(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] OnGameResume called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameResume");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnGameResume at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }

    game_paused = 0;
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameTouchEvent(JNIEnv *env, jobject obj, int x, int y, int action) {
    printf("[SAFE-JNI] Touch event - STUB MODE: x=%d, y=%d, action=%d\n", x, y, action);

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameTouchEvent");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnGameTouchEvent at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameBack(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] OnGameBack called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameBack");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnGameBack at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

// Resource management - SAFE STUBS
JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[SAFE-JNI] SetResourcePath called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real SetResourcePath at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[SAFE-JNI] SetFilePath called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real SetFilePath at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] OnLibraryInitialized called - STUB MODE\n");
    printf("[SAFE-JNI] env=%p, obj=%p\n", env, obj);

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnLibraryInitialized at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
            printf("[SAFE-JNI] This is the function that was crashing!\n");
        }
    }

    printf("[SAFE-JNI] OnLibraryInitialized completed safely (stub mode)\n");
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete(JNIEnv *env, jobject obj, int sound_id) {
    printf("[SAFE-JNI] OnPlaySoundComplete: sound_id=%d - STUB MODE\n", sound_id);

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real OnPlaySoundComplete at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onCashUpdate(JNIEnv *env, jobject obj, int cash_amount) {
    printf("[SAFE-JNI] onCashUpdate: cash=%d - STUB MODE\n", cash_amount);
    // Monetization function - safe to stub
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onLanguage(JNIEnv *env, jobject obj, jstring language) {
    printf("[SAFE-JNI] onLanguage called - STUB MODE\n");
    // Language setting - safe to stub
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onHotDogCreate(JNIEnv *env, jobject obj) {
    printf("[SAFE-JNI] onHotDogCreate called - STUB MODE\n");

    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_onHotDogCreate");
        if (func_addr) {
            printf("[SAFE-JNI] FOUND real onHotDogCreate at: 0x%08x\n", func_addr);
            printf("[SAFE-JNI] (NOT calling it yet - stub mode)\n");
        }
    }
}

// Utility functions
int is_game_initialized() {
    return game_initialized;
}

int is_game_paused() {
    return game_paused;
}

int get_game_phase() {
    return menu_phase;
}
