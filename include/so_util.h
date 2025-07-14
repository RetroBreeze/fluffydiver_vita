/*
 * so_util.h - Enhanced header for complete SO loading functionality
 * Based on successful PS Vita Android port analysis
 * Includes all components from Modern Combat 3, Mass Effect, Galaxy on Fire 2, The Conduit
 */

#ifndef __SO_UTIL_H__
#define __SO_UTIL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// ===== CORE SO-LOADER STRUCTURES =====

typedef struct {
    char *symbol;
    uintptr_t func;
} DynLibFunction;

typedef struct so_module {
    void *base;
    size_t size;
    void *dynsym;
    void *dynstr;
    void *hash;
    size_t dynsym_num;
    void *text_base;
    size_t text_size;
    void *rel;           // DT_REL
    size_t rel_size;     // DT_RELSZ
    void *plt_rel;       // DT_JMPREL
    size_t plt_rel_size; // DT_PLTRELSZ
} so_module;

// ===== CORE SO-LOADER FUNCTIONS =====
int so_load(so_module *mod, const char *path, uintptr_t load_addr);
int so_relocate(so_module *mod);
int so_resolve(so_module *mod, DynLibFunction *funcs, size_t num_funcs, int strict);
void so_flush_caches(so_module *mod);
int so_initialize(so_module *mod);
uintptr_t so_symbol(so_module *mod, const char *symbol);
void hook_addr(uintptr_t addr, uintptr_t dst);

// ===== SYMBOL ANALYSIS FUNCTIONS =====
int so_analyze_and_try_symbols(so_module *mod, void *fake_env, void *fake_context);
int so_analyze_and_try_symbols_with_hang_detection(so_module *mod, void *fake_env, void *fake_context,
                                                   volatile int *function_returned_ptr,
                                                   int (*hang_detection_func)(SceSize, void*));
int so_find_real_entry_points(so_module *mod, void *fake_env, void *fake_context);

// ===== UTILITY FUNCTIONS =====
int ret0(void);
int ret1(void);
int retminus1(void);
void *retNULL(void);

// ===== DEBUG FUNCTIONS =====
void debugPrintf(const char *fmt, ...);
void print_symbol_resolution_stats(void);

// ===== FILE I/O HOOK FUNCTIONS =====
FILE *fopen_hook(const char *filename, const char *mode);
int open_hook(const char *pathname, int flags, mode_t mode);

// ===== MEMORY ALLOCATION (Safe Versions) =====
void *malloc_safe(size_t size);
void *calloc_safe(size_t nmemb, size_t size);
void *realloc_safe(void *ptr, size_t size);

// ===== ENHANCED PTHREAD FUNCTIONS =====
int pthread_mutex_init_fake(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy_fake(pthread_mutex_t *mutex);
int pthread_mutex_lock_fake(pthread_mutex_t *mutex);
int pthread_mutex_unlock_fake(pthread_mutex_t *mutex);

// ===== TIME FUNCTIONS =====
int gettimeofday_vita(struct timeval *tv, void *tz);

// ===== JNI FUNCTIONS =====
int JNI_OnLoad(void *vm, void *reserved);

// ===== ANDROID API DECLARATIONS =====

// Android system initialization
void android_api_init(void);
void android_api_cleanup(void);

// Android context API
void *android_getApplicationContext(void);
void *android_getSystemService(void *context, const char *service);
void *android_getAssets(void *context);
void *android_getPackageName(void *context);
void *android_getExternalFilesDir(void *context, void *type);
void *android_getFilesDir(void *context);
void *android_getCacheDir(void *context);
int android_getVersionCode(void *context);
void *android_getVersionName(void *context);

// Android asset manager API
void *android_asset_open(void *asset_manager, const char *filename, int mode);
int android_asset_read(void *asset, void *buffer, int size);
int android_asset_seek(void *asset, int offset, int whence);
int android_asset_getLength(void *asset);
void android_asset_close(void *asset);
int android_asset_getRemainingLength(void *asset);

// Android display API
void android_getDisplayMetrics(void *context, int *width, int *height, float *density);
int android_getOrientation(void *context);
int android_getScreenWidth(void *context);
int android_getScreenHeight(void *context);
float android_getScreenDensity(void *context);

// Android audio API
int android_audio_getSampleRate(void *audio_manager);
int android_audio_getFramesPerBuffer(void *audio_manager);
int android_audio_getChannelCount(void *audio_manager);
void android_audio_setVolume(void *audio_manager, float volume);
float android_audio_getVolume(void *audio_manager);

// Android input API
int android_input_isTouchEnabled(void *context);
int android_input_isGyroscopeEnabled(void *context);
int android_input_isAccelerometerEnabled(void *context);
void android_input_setTouchEnabled(void *context, int enabled);
void android_input_setGyroscopeEnabled(void *context, int enabled);

// Android sensor API
typedef struct {
    float x, y, z;
    uint64_t timestamp;
} AndroidSensorEvent;

void android_sensor_getAccelerometer(AndroidSensorEvent *event);
void android_sensor_getGyroscope(AndroidSensorEvent *event);

// Android network API
int android_network_isConnected(void *connectivity_manager);
int android_network_isWifiConnected(void *connectivity_manager);
int android_network_isMobileConnected(void *connectivity_manager);

// Android storage API
long long android_storage_getFreeSpace(void *context);
long long android_storage_getTotalSpace(void *context);

// Android preferences API
void *android_getSharedPreferences(void *context, const char *name, int mode);
int android_prefs_getInt(void *prefs, const char *key, int default_value);
void android_prefs_putInt(void *prefs, const char *key, int value);
void *android_prefs_getString(void *prefs, const char *key, const char *default_value);
void android_prefs_putString(void *prefs, const char *key, const char *value);

// Android lifecycle API
void android_onCreate(void *activity);
void android_onStart(void *activity);
void android_onResume(void *activity);
void android_onPause(void *activity);
void android_onStop(void *activity);
void android_onDestroy(void *activity);

// Android utility API
void android_vibrate(void *context, int milliseconds);
void android_startActivity(void *context, const char *action, const char *data);
int android_log_print(int priority, const char *tag, const char *message);
void android_runOnUiThread(void *context, void *runnable);
void android_set_abort_message(const char *msg);

// Android bitmap API
void *android_bitmap_createBitmap(int width, int height, int config);
void android_bitmap_recycle(void *bitmap);
int android_bitmap_getWidth(void *bitmap);
int android_bitmap_getHeight(void *bitmap);

// ===== ANDROID LOGGING API =====
int __android_log_print(int prio, const char *tag, const char *fmt, ...);
int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args);
int __android_log_write(int prio, const char *tag, const char *text);

// ===== ANDROID SYSTEM PROPERTIES =====
int __system_property_get(const char *name, char *value);

// ===== ANDROID LOOPER =====
void *ALooper_forThread(void);
void *ALooper_prepare(int opts);
int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData);

// ===== GLOBAL VARIABLES =====
extern void *fake_env;
extern void *fake_context;
extern DynLibFunction default_dynlib[];
extern size_t default_dynlib_size;

// ===== GAME INITIALIZATION FUNCTIONS =====
int check_kubridge(void);
int file_exists(const char *path);
void fatal_error(const char *fmt, ...);
void patch_game(void);
void init_android_environment(void);
int call_game_entry_point(void);

// ===== ENHANCED ERROR HANDLING =====
#define ANDROID_LOG_UNKNOWN 0
#define ANDROID_LOG_DEFAULT 1
#define ANDROID_LOG_VERBOSE 2
#define ANDROID_LOG_DEBUG   3
#define ANDROID_LOG_INFO    4
#define ANDROID_LOG_WARN    5
#define ANDROID_LOG_ERROR   6
#define ANDROID_LOG_FATAL   7
#define ANDROID_LOG_SILENT  8

// ===== ASSET MANAGER CONSTANTS =====
#define AASSET_MODE_UNKNOWN   0
#define AASSET_MODE_RANDOM    1
#define AASSET_MODE_STREAMING 2
#define AASSET_MODE_BUFFER    3

// ===== SCREEN ORIENTATION CONSTANTS =====
#define ANDROID_ORIENTATION_UNDEFINED  0
#define ANDROID_ORIENTATION_PORTRAIT   1
#define ANDROID_ORIENTATION_LANDSCAPE  2

// ===== BITMAP CONFIG CONSTANTS =====
#define ANDROID_BITMAP_FORMAT_NONE      0
#define ANDROID_BITMAP_FORMAT_RGBA_8888 1
#define ANDROID_BITMAP_FORMAT_RGB_565   4
#define ANDROID_BITMAP_FORMAT_RGBA_4444 7
#define ANDROID_BITMAP_FORMAT_A_8       8

// ===== SUCCESSFUL PORT COMPATIBILITY MACROS =====
// These ensure compatibility with patterns from successful ports

// Modern Combat 3 compatibility
#define MC3_COMPAT_INIT() android_api_init()
#define MC3_ASSET_OPEN(mgr, file, mode) android_asset_open(mgr, file, mode)

// Mass Effect compatibility
#define MASSEFFECT_COMPAT_INIT() android_api_init()
#define MASSEFFECT_SENSOR_INIT() android_input_setGyroscopeEnabled(NULL, 0)

// Galaxy on Fire 2 compatibility
#define GOF2_COMPAT_INIT() android_api_init()
#define GOF2_STORAGE_CHECK() android_storage_getFreeSpace(NULL)

// The Conduit compatibility
#define CONDUIT_COMPAT_INIT() android_api_init()
#define CONDUIT_DISPLAY_INIT() android_getDisplayMetrics(NULL, NULL, NULL, NULL)

// ===== VERSION INFORMATION =====
#define SO_UTIL_VERSION_MAJOR 1
#define SO_UTIL_VERSION_MINOR 4
#define SO_UTIL_VERSION_PATCH 0
#define SO_UTIL_VERSION_STRING "1.4.0-enhanced"

// ===== BUILD CONFIGURATION =====
#ifdef DEBUG
#define SO_DEBUG_PRINT(fmt, ...) debugPrintf("[SO_DEBUG] " fmt, ##__VA_ARGS__)
#else
#define SO_DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

// ===== MEMORY MANAGEMENT =====
#define SO_MALLOC(size) malloc_safe(size)
#define SO_CALLOC(nmemb, size) calloc_safe(nmemb, size)
#define SO_REALLOC(ptr, size) realloc_safe(ptr, size)
#define SO_FREE(ptr) do { if(ptr) { free(ptr); ptr = NULL; } } while(0)

// ===== ERROR CODES =====
#define SO_SUCCESS           0
#define SO_ERROR_INVALID    -1
#define SO_ERROR_NOT_FOUND  -2
#define SO_ERROR_MEMORY     -3
#define SO_ERROR_IO         -4
#define SO_ERROR_SYMBOL     -5
#define SO_ERROR_RELOCATE   -6
#define SO_ERROR_INITIALIZE -7

// ===== FUNCTION RESULT CHECKING =====
#define SO_CHECK(expr) do { \
int __result = (expr); \
if (__result < 0) { \
    debugPrintf("SO_CHECK failed: %s returned %d at %s:%d\n", \
    #expr, __result, __FILE__, __LINE__); \
    return __result; \
} \
} while(0)

#define SO_CHECK_FATAL(expr, msg) do { \
int __result = (expr); \
if (__result < 0) { \
    fatal_error("SO_CHECK_FATAL: %s - %s returned %d at %s:%d", \
    msg, #expr, __result, __FILE__, __LINE__); \
} \
} while(0)

#endif // __SO_UTIL_H__
