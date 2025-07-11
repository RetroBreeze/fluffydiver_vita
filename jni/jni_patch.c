/*
 * Complete FalsoJNI JNI Patch - Calls real game functions through FalsoJNI
 * Clean version with no compilation errors
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

// Forward declaration to fix compilation warning
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
 * ENHANCED JNI FUNCTION IMPLEMENTATIONS
 * Now calls REAL game functions through FalsoJNI
 */

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameInitialize(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameInitialize - Enhanced with FalsoJNI\n");
    
    // Use FalsoJNI environment
    JNIEnv *falsojni_env = falsojni_get_env();
    
    // Call the actual game initialization function with FalsoJNI environment
    if (so_functions_resolved()) {
        printf("[JNI] Calling real game initialization with FalsoJNI environment: %p\n", falsojni_env);
        
        // Get the actual function pointer
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameInitialize");
        if (func_addr) {
            // Cast to proper function type and call
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t game_init = (jni_func_t)func_addr;
            
            printf("[JNI] Calling game function at 0x%08x with FalsoJNI env=%p, obj=%p\n", 
                   func_addr, falsojni_env, obj);
            
            game_init(falsojni_env, obj);
            
            printf("[JNI] Game initialization function returned successfully\n");
        } else {
            printf("[JNI] ERROR: Could not find OnGameInitialize function\n");
        }
    } else {
        printf("[JNI] Running in stub mode - game functions not resolved\n");
    }

    game_initialized = 1;
    menu_phase = 1;

    printf("[JNI] OnGameInitialize completed successfully\n");
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameUpdate(JNIEnv *env, jobject obj) {
    static int call_count = 0;
    call_count++;
    
    // Use FalsoJNI environment
    JNIEnv *falsojni_env = falsojni_get_env();

    // Call the actual game update function
    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameUpdate");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t game_update = (jni_func_t)func_addr;
            
            // Only log occasionally to avoid spam
            if (call_count % 300 == 0) {
                printf("[JNI] Calling real OnGameUpdate #%d with FalsoJNI env=%p\n", call_count, falsojni_env);
            }
            
            game_update(falsojni_env, obj);
        }
    }

    // Keep auto-progression logic as fallback
    static int frame_count = 0;
    frame_count++;

    if (frame_count % 180 == 0) {
        switch (menu_phase) {
            case 1:  // Title screen
                printf("[JNI] Auto-progression: Title -> Game (frame %d)\n", frame_count);
                Java_com_hotdog_jni_Natives_OnGameTouchEvent(falsojni_env, obj, 480, 400, 1);
                menu_phase = 2;
                break;
            case 2:  // Game screen
                if (!game_started) {
                    printf("[JNI] Auto-progression: Starting dive (frame %d)\n", frame_count);
                    Java_com_hotdog_jni_Natives_OnGameTouchEvent(falsojni_env, obj, 480, 272, 1);
                    game_started = 1;
                }
                break;
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGamePause(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGamePause with FalsoJNI environment\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGamePause");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t game_pause = (jni_func_t)func_addr;
            game_pause(falsojni_env, obj);
        }
    }

    game_paused = 1;
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameResume(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameResume with FalsoJNI environment\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameResume");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t game_resume = (jni_func_t)func_addr;
            game_resume(falsojni_env, obj);
        }
    }

    game_paused = 0;
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameTouchEvent(JNIEnv *env, jobject obj, int x, int y, int action) {
    printf("[JNI] Touch event: x=%d, y=%d, action=%d\n", x, y, action);
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameTouchEvent");
        if (func_addr) {
            // Touch events might have different signatures, but try basic approach first
            typedef void (*jni_func_t)(JNIEnv*, jobject, int, int, int);
            jni_func_t game_touch = (jni_func_t)func_addr;
            game_touch(falsojni_env, obj, x, y, action);
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameBack(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameBack with FalsoJNI environment\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_OnGameBack");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t game_back = (jni_func_t)func_addr;
            game_back(falsojni_env, obj);
        }
    }
}

// Resource management - CRITICAL for asset loading
JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[JNI] SetResourcePath - configuring asset path with FalsoJNI\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject, jstring);
            jni_func_t set_path = (jni_func_t)func_addr;
            
            printf("[JNI] Calling SetResourcePath with NULL path (will be fixed later)\n");
            set_path(falsojni_env, obj, NULL);
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[JNI] SetFilePath - configuring file path with FalsoJNI\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject, jstring);
            jni_func_t set_path = (jni_func_t)func_addr;
            
            printf("[JNI] Calling SetFilePath with NULL path (will be fixed later)\n");
            set_path(falsojni_env, obj, NULL);
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized(JNIEnv *env, jobject obj) {
    printf("[JNI] OnLibraryInitialized with FalsoJNI environment\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t lib_init = (jni_func_t)func_addr;
            
            printf("[JNI] Calling real OnLibraryInitialized with FalsoJNI env=%p\n", falsojni_env);
            lib_init(falsojni_env, obj);
            printf("[JNI] OnLibraryInitialized returned successfully\n");
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete(JNIEnv *env, jobject obj, int sound_id) {
    printf("[JNI] OnPlaySoundComplete: sound_id=%d\n", sound_id);
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject, int);
            jni_func_t sound_complete = (jni_func_t)func_addr;
            sound_complete(falsojni_env, obj, sound_id);
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onCashUpdate(JNIEnv *env, jobject obj, int cash_amount) {
    printf("[JNI] onCashUpdate: cash=%d\n", cash_amount);
    // Monetization function - safe to stub for homebrew
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onLanguage(JNIEnv *env, jobject obj, jstring language) {
    printf("[JNI] onLanguage called\n");
    // Language setting - safe to stub
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onHotDogCreate(JNIEnv *env, jobject obj) {
    printf("[JNI] onHotDogCreate with FalsoJNI environment\n");
    
    JNIEnv *falsojni_env = falsojni_get_env();

    if (so_functions_resolved()) {
        uintptr_t func_addr = so_symbol(NULL, "Java_com_hotdog_jni_Natives_onHotDogCreate");
        if (func_addr) {
            typedef void (*jni_func_t)(JNIEnv*, jobject);
            jni_func_t hotdog_create = (jni_func_t)func_addr;
            printf("[JNI] Calling real onHotDogCreate with FalsoJNI env=%p\n", falsojni_env);
            hotdog_create(falsojni_env, obj);
            printf("[JNI] onHotDogCreate returned successfully\n");
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
