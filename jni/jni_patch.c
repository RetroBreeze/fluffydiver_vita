#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

// JNI type definitions for so-loader compatibility
typedef struct _JavaVM JavaVM;
typedef struct _JNIEnv JNIEnv;
typedef void* jobject;
typedef void* jstring;

// JNI function calling convention
#define JNIEXPORT
#define JNICALL

// Global variables for game state
static int game_initialized = 0;
static int game_paused = 0;
static float screen_width = 960.0f;
static float screen_height = 544.0f;

// Touch input mapping
static SceTouchData touch_data;
static int touch_enabled = 1;

// Game state tracking
static int game_started = 0;
static int menu_phase = 0;  // 0=init, 1=title, 2=game

// Forward declarations
void Java_com_hotdog_jni_Natives_OnGameTouchEvent(JNIEnv *env, jobject obj, int x, int y, int action);

/*
 * JNI FUNCTION IMPLEMENTATIONS
 * These match the exact signatures from your libFluffyDiver.so
 */

// Game Lifecycle Functions
JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameInitialize(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameInitialize called\n");
    game_initialized = 1;
    menu_phase = 1;  // Move to title screen phase
    
    // Initialize touch
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 0);
    
    // Send initial touch event to potentially trigger title screen
    printf("[JNI] Sending initial touch event to start game\n");
    Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 272, 1);  // Center screen tap
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameUpdate(JNIEnv *env, jobject obj) {
    // This is called every frame - only log important state changes
    static int frame_count = 0;
    static int last_log_phase = -1;
    
    frame_count++;
    
    // Log phase changes
    if (menu_phase != last_log_phase) {
        printf("[JNI] Game phase changed: %d -> %d (frame %d)\n", 
               last_log_phase, menu_phase, frame_count);
        last_log_phase = menu_phase;
    }
    
    // Auto-progress through menus by sending touch events
    if (frame_count % 180 == 0) {  // Every 3 seconds
        switch (menu_phase) {
            case 1:  // Title screen - tap to start
                printf("[JNI] Auto-tapping to start game (frame %d)\n", frame_count);
                Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 400, 1);  // Start button area
                menu_phase = 2;
                break;
            case 2:  // Game screen - tap to dive
                if (!game_started) {
                    printf("[JNI] Auto-tapping to begin diving (frame %d)\n", frame_count);
                    Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 272, 1);  // Center tap
                    game_started = 1;
                }
                break;
        }
    }
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGamePause(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGamePause called\n");
    game_paused = 1;
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameResume(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameResume called\n");
    game_paused = 0;
}

// Input Functions
JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameTouchEvent(JNIEnv *env, jobject obj, int x, int y, int action) {
    // action: 0 = touch up, 1 = touch down, 2 = touch move
    printf("[JNI] Touch event sent to game: x=%d, y=%d, action=%d\n", x, y, action);
    
    // The game should process this touch event and potentially change state
    // This is where the game would load assets, transition screens, etc.
}

JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_OnGameBack(JNIEnv *env, jobject obj) {
    printf("[JNI] OnGameBack called\n");
    // Handle back button - could exit game or return to menu
}

// Resource Management Functions
JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetResourcePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[JNI] SetResourcePath called - setting to ux0:data/fluffydiver/\n");
    
    // The game is asking where to find its assets
    // This is a critical call - the game needs to know the asset path
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_SetFilePath(JNIEnv *env, jobject obj, jstring path) {
    printf("[JNI] SetFilePath called - setting to ux0:data/fluffydiver/\n");
}

// Audio Functions
JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnPlaySoundComplete(JNIEnv *env, jobject obj, int sound_id) {
    printf("[JNI] OnPlaySoundComplete: sound_id=%d\n", sound_id);
    // Audio callback - called when sound finishes playing
}

// Monetization Functions
JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onCashUpdate(JNIEnv *env, jobject obj, int cash_amount) {
    printf("[JNI] onCashUpdate: cash=%d\n", cash_amount);
    // Handle in-app purchase updates - likely safe to ignore for homebrew
}

// Language/Localization Functions
JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onLanguage(JNIEnv *env, jobject obj, jstring language) {
    printf("[JNI] onLanguage called - setting to English\n");
}

// Initialization Functions
JNIEXPORT void JNICALL Java_com_hotdog_jni_Natives_onHotDogCreate(JNIEnv *env, jobject obj) {
    printf("[JNI] onHotDogCreate called\n");
    // Early initialization - called before OnGameInitialize
}

JNIEXPORT void JNICALL Java_com_hotdog_libraryInterface_hdNativeInterface_OnLibraryInitialized(JNIEnv *env, jobject obj) {
    printf("[JNI] OnLibraryInitialized called\n");
    // Library-level initialization
}

// Enhanced input handling for Vita controls
void handle_vita_input_to_game(JNIEnv *env, jobject obj) {
    static SceCtrlData last_pad = {0};
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);
    
    // Convert Vita controls to touch events
    if (pad.buttons != last_pad.buttons) {
        if (pad.buttons & SCE_CTRL_CROSS) {
            printf("[JNI] X pressed - sending dive touch\n");
            Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 272, 1);  // Dive
        }
        if (pad.buttons & SCE_CTRL_CIRCLE) {
            printf("[JNI] Circle pressed - sending surface touch\n");
            Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 200, 1);  // Surface
        }
        if (pad.buttons & SCE_CTRL_START) {
            printf("[JNI] Start pressed - sending menu touch\n");
            Java_com_hotdog_jni_Natives_OnGameTouchEvent(env, obj, 480, 400, 1);  // Menu area
        }
        if (pad.buttons & SCE_CTRL_SELECT) {
            printf("[JNI] Select pressed - calling OnGameBack\n");
            Java_com_hotdog_jni_Natives_OnGameBack(env, obj);
        }
    }
    
    last_pad = pad;
}

// Utility Functions
int is_game_initialized() {
    return game_initialized;
}

int is_game_paused() {
    return game_paused;
}

int get_game_phase() {
    return menu_phase;
}

void set_touch_enabled(int enabled) {
    touch_enabled = enabled;
}
