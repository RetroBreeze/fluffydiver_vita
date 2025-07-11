#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

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

// JNI type definitions (minimal for so-loader)
typedef struct _JavaVM JavaVM;
typedef struct _JNIEnv JNIEnv;
typedef void* jobject;
typedef void* jstring;

// JNI environment globals
static JavaVM *g_jvm = NULL;
static JNIEnv *g_env = NULL;

// Game state
static int game_running = 1;
static so_module so_mod;

// Debug logging
static FILE *debug_log = NULL;

// Load address for so-loader
#define LOAD_ADDRESS 0x8000000

// Function prototypes from our JNI stubs
extern void Java_com_hotdog_jni_Natives_OnGameInitialize(JNIEnv *env, jobject obj);
extern void Java_com_hotdog_jni_Natives_OnGameUpdate(JNIEnv *env, jobject obj);
extern void Java_com_hotdog_jni_Natives_OnGamePause(JNIEnv *env, jobject obj);
extern void Java_com_hotdog_jni_Natives_OnGameResume(JNIEnv *env, jobject obj);
extern int is_game_initialized();
extern int is_game_paused();

// Function prototypes from android_stubs.c
extern void create_asset_debug_files();

/*
 * DEBUG LOGGING FUNCTIONS
 */
static void init_debug_log() {
    // Create debug directory if it doesn't exist
    sceIoMkdir("ux0:data/fluffydiver", 0777);
    
    debug_log = fopen("ux0:data/fluffydiver/debug.log", "w");
    if (debug_log) {
        SceDateTime time;
        sceRtcGetCurrentClock(&time, 0);
        
        fprintf(debug_log, "=== Fluffy Diver Debug Log ===\n");
        fprintf(debug_log, "Started: %04d-%02d-%02d %02d:%02d:%02d\n",
                time.year, time.month, time.day, time.hour, time.minute, time.second);
        fprintf(debug_log, "Build: %s %s\n", __DATE__, __TIME__);
        fprintf(debug_log, "================================\n\n");
        fflush(debug_log);
        
        printf("Debug log initialized: ux0:data/fluffydiver/debug.log\n");
    } else {
        printf("WARNING: Could not create debug log file\n");
    }
}

static void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Print to console
    vprintf(fmt, args);
    
    // Also log to file with timestamp
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

static void debug_opengl_state() {
    static int last_report_frame = -1000;
    static int frame_count = 0;
    
    frame_count++;
    
    // Only report every 5 seconds to avoid spam
    if (frame_count - last_report_frame > 300) {
        last_report_frame = frame_count;
        
        // Get OpenGL state
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        GLfloat clear_color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
        
        GLenum error = glGetError();
        
        debug_printf("=== OpenGL State Report ===\n");
        debug_printf("Viewport: %d, %d, %d, %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
        debug_printf("Clear Color: %.2f, %.2f, %.2f, %.2f\n", 
                    clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        debug_printf("GL Error: 0x%04X\n", error);
        
        // Check if any textures are bound
        GLint texture_2d;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_2d);
        debug_printf("Bound Texture 2D: %d\n", texture_2d);
        
        // Check matrix mode
        GLint matrix_mode;
        glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
        debug_printf("Matrix Mode: 0x%04X\n", matrix_mode);
        
        debug_printf("=== End OpenGL State ===\n");
    }
}

/*
 * VITA SYSTEM INITIALIZATION
 */
static void init_vita_systems() {
    debug_printf("Initializing Vita systems...\n");
    
    // Initialize VitaGL
    debug_printf("Initializing VitaGL...\n");
    vglInit(0x1000000); // 16MB for VitaGL
    vglWaitVblankStart(GL_FALSE);
    
    // Clear screen to blue so we know VitaGL is working
    debug_color_change("BLUE - VitaGL Init", 0.0f, 0.0f, 1.0f);
    
    // Initialize input
    debug_printf("Initializing input systems...\n");
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 0);
    
    debug_printf("Vita systems initialized successfully\n");
}

/*
 * JNI ENVIRONMENT SETUP
 */
static int setup_jni_environment() {
    debug_printf("Setting up JNI environment...\n");
    
    // For so-loader, we don't need a real JNI environment
    // Just set up dummy pointers that won't crash
    g_jvm = (JavaVM*)malloc(sizeof(void*));
    g_env = (JNIEnv*)malloc(sizeof(void*));
    
    if (!g_jvm || !g_env) {
        debug_printf("ERROR: Failed to allocate JNI environment\n");
        return -1;
    }
    
    debug_printf("JNI environment ready\n");
    return 0;
}

/*
 * ASSET VERIFICATION AND DEBUGGING
 */
static int verify_and_debug_assets() {
    debug_printf("Verifying game assets...\n");
    
    // Check if assets directory exists
    SceUID dir = sceIoDopen("ux0:data/fluffydiver/");
    if (dir < 0) {
        debug_printf("ERROR: Assets directory not found! (error: 0x%08X)\n", dir);
        debug_printf("Please extract game assets to: ux0:data/fluffydiver/\n");
        return -1;
    }
    
    // Count files in assets directory
    SceIoDirent entry;
    int file_count = 0;
    debug_printf("Assets found:\n");
    while (sceIoDread(dir, &entry) > 0) {
        debug_printf("  %s (%s, %d bytes)\n", 
                   entry.d_name, 
                   SCE_S_ISDIR(entry.d_stat.st_mode) ? "DIR" : "FILE",
                   (int)entry.d_stat.st_size);
        file_count++;
    }
    sceIoDclose(dir);
    debug_printf("Total assets: %d files\n", file_count);
    
    // Check for .so file
    SceUID so_file = sceIoOpen("app0:lib/libFluffyDiver.so", SCE_O_RDONLY, 0);
    if (so_file < 0) {
        debug_printf("ERROR: libFluffyDiver.so not found! (error: 0x%08X)\n", so_file);
        debug_printf("Please place libFluffyDiver.so in VPK at: lib/\n");
        return -1;
    }
    
    // Get file size for verification
    SceOff size = sceIoLseek(so_file, 0, SCE_SEEK_END);
    sceIoClose(so_file);
    debug_printf("Found libFluffyDiver.so (%d bytes)\n", (int)size);
    
    // Create asset debug files
    debug_printf("Creating asset debug files...\n");
    create_asset_debug_files();
    
    debug_printf("Asset verification completed successfully\n");
    return 0;
}

/*
 * GAME LOOP - WITH ASSET AND OPENGL DEBUGGING
 */
static void game_loop() {
    debug_printf("Starting game loop...\n");
    
    // 1. Show green screen to indicate we reached the game loop
    debug_color_change("GREEN - Game Loop Started", 0.0f, 1.0f, 0.0f);
    sceKernelDelayThread(500000); // 0.5 second
    
    // 2. Show red screen to indicate we're about to load the game
    debug_color_change("RED - About to Initialize", 1.0f, 0.0f, 0.0f);
    sceKernelDelayThread(500000); // 0.5 second
    
    // 3. Initialize the game
    debug_printf("Calling OnGameInitialize...\n");
    Java_com_hotdog_jni_Natives_OnGameInitialize(g_env, NULL);
    debug_printf("OnGameInitialize returned\n");
    
    // 4. Show white screen to indicate JNI call completed
    debug_color_change("WHITE - JNI Call Complete", 1.0f, 1.0f, 1.0f);
    sceKernelDelayThread(1000000); // 1 second
    
    // 5. Check if the game actually initialized
    if (is_game_initialized()) {
        debug_printf("Game reports as initialized successfully\n");
        // Show purple screen for successful init
        debug_color_change("PURPLE - Game Initialized", 0.5f, 0.0f, 1.0f);
        sceKernelDelayThread(1000000); // 1 second
    } else {
        debug_printf("WARNING: Game NOT initialized after JNI call\n");
        // Show yellow screen for failed init
        debug_color_change("YELLOW - Init Failed", 1.0f, 1.0f, 0.0f);
        sceKernelDelayThread(2000000); // 2 seconds
    }
    
    // 6. Start interactive testing phase
    debug_printf("Starting main game loop with asset debugging...\n");
    debug_printf("Controls: X=Red, Circle=Green, Triangle=Blue, Square=Yellow, No buttons=Cyan\n");
    debug_printf("Press Start+Select to exit\n");
    
    int frame_count = 0;
    int last_button_state = 0;
    
    // Main game loop - WITH ASSET AND OPENGL DEBUGGING
    while (game_running) {
        // Handle Vita system input
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        
        // Check for exit condition (Start + Select)
        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) {
            debug_printf("Exit combination pressed (Start + Select)\n");
            game_running = 0;
            break;
        }
        
        // Button testing - change colors based on input
        if (pad.buttons & SCE_CTRL_CROSS) {
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red when X pressed
            if (!(last_button_state & SCE_CTRL_CROSS)) {
                debug_printf("INPUT: X button pressed (RED)\n");
            }
        } else if (pad.buttons & SCE_CTRL_CIRCLE) {
            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // Green when Circle pressed
            if (!(last_button_state & SCE_CTRL_CIRCLE)) {
                debug_printf("INPUT: Circle button pressed (GREEN)\n");
            }
        } else if (pad.buttons & SCE_CTRL_TRIANGLE) {
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);  // Blue when Triangle pressed
            if (!(last_button_state & SCE_CTRL_TRIANGLE)) {
                debug_printf("INPUT: Triangle button pressed (BLUE)\n");
            }
        } else if (pad.buttons & SCE_CTRL_SQUARE) {
            glClearColor(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow when Square pressed
            if (!(last_button_state & SCE_CTRL_SQUARE)) {
                debug_printf("INPUT: Square button pressed (YELLOW)\n");
            }
        } else if (pad.buttons & SCE_CTRL_LTRIGGER) {
            glClearColor(1.0f, 0.0f, 1.0f, 1.0f);  // Magenta when L pressed
            if (!(last_button_state & SCE_CTRL_LTRIGGER)) {
                debug_printf("INPUT: L Trigger pressed (MAGENTA)\n");
            }
        } else if (pad.buttons & SCE_CTRL_RTRIGGER) {
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);  // Gray when R pressed
            if (!(last_button_state & SCE_CTRL_RTRIGGER)) {
                debug_printf("INPUT: R Trigger pressed (GRAY)\n");
            }
        } else {
            glClearColor(0.0f, 1.0f, 1.0f, 1.0f);  // Cyan when no buttons
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        vglSwapBuffers(GL_FALSE);
        
        // Store button state for next frame
        last_button_state = pad.buttons;
        
        // Call game update every frame
        if (is_game_initialized() && !is_game_paused()) {
            // Every 10th frame, log that we're calling OnGameUpdate
            if (frame_count % 10 == 0) {
                debug_printf("Calling OnGameUpdate (frame %d)\n", frame_count);
            }
            
            // Call the game's update function
            Java_com_hotdog_jni_Natives_OnGameUpdate(g_env, NULL);
            
            // Debug OpenGL state periodically
            debug_opengl_state();
            
            // Check if the game update caused any issues
            if (frame_count % 10 == 0) {
                debug_printf("OnGameUpdate returned successfully (frame %d)\n", frame_count);
            }
        }
        
        // Print frame info periodically
        frame_count++;
        if (frame_count % 300 == 0) {  // Every 5 seconds at 60fps
            debug_printf("Frame %d - Game running smoothly\n", frame_count);
            debug_printf("Game state: initialized=%d, paused=%d\n", 
                        is_game_initialized(), is_game_paused());
        }
        
        // Cap to ~60fps
        usleep(16666);
    }
    
    debug_printf("Game loop ended after %d frames\n", frame_count);
}

/*
 * MAIN FUNCTION
 */
int main() {
    // Initialize debug logging FIRST
    init_debug_log();
    
    debug_printf("=== Fluffy Diver PS Vita Port ===\n");
    debug_printf("Starting initialization...\n");
    
    // Initialize Vita systems
    init_vita_systems();
    
    // Verify assets and create debug files
    if (verify_and_debug_assets() < 0) {
        debug_printf("Asset verification failed. Exiting in 5 seconds...\n");
        debug_color_change("RED - Asset Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000); // 5 seconds
        goto cleanup;
    }
    
    // Setup JNI environment
    if (setup_jni_environment() < 0) {
        debug_printf("JNI setup failed. Exiting in 5 seconds...\n");
        debug_color_change("RED - JNI Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000); // 5 seconds
        goto cleanup;
    }
    
    // Load the game's .so file
    debug_printf("Loading libFluffyDiver.so...\n");
    int ret = so_file_load(&so_mod, "app0:lib/libFluffyDiver.so", LOAD_ADDRESS);
    if (ret < 0) {
        debug_printf("ERROR: Failed to load .so file (error: %d)\n", ret);
        debug_color_change("RED - SO Load Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000); // 5 seconds
        goto cleanup;
    }
    debug_printf("SO file loaded successfully\n");
    
    // Resolve symbols
    debug_printf("Resolving symbols...\n");
    ret = so_relocate(&so_mod);
    if (ret < 0) {
        debug_printf("ERROR: Failed to relocate symbols (error: %d)\n", ret);
        debug_color_change("RED - Relocation Error", 1.0f, 0.0f, 0.0f);
        sceKernelDelayThread(5000000); // 5 seconds
        goto cleanup;
    }
    debug_printf("Symbol relocation completed\n");
    
    // Initialize the loaded module
    debug_printf("Initializing loaded module...\n");
    so_initialize(&so_mod);
    debug_printf("Module initialization completed\n");
    
    // Start game
    debug_printf("Starting game main loop...\n");
    game_loop();
    
cleanup:
    debug_printf("Cleaning up...\n");
    
    // Cleanup JNI
    if (g_env) {
        free(g_env);
        debug_printf("JNI environment freed\n");
    }
    if (g_jvm) {
        free(g_jvm);
        debug_printf("JNI VM freed\n");
    }
    
    // Cleanup VitaGL
    vglEnd();
    debug_printf("VitaGL terminated\n");
    
    // Close debug log
    if (debug_log) {
        debug_printf("Port terminated normally\n");
        fclose(debug_log);
        debug_log = NULL;
    }
    
    printf("Exiting...\n");
    sceKernelExitProcess(0);
    return 0;
}

/*
 * REQUIRED WRAPPERS FOR SO-LOADER
 */
void *__wrap_memcpy(void *dest, const void *src, size_t n) {
    return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n) {
    return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n) {
    return sceClibMemset(s, c, n);
}
