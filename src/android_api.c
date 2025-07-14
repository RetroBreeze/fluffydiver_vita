/*
 * android_api.c - Complete Android API implementation for Fluffy Diver
 * Based on successful port analysis: Modern Combat 3, Mass Effect, Galaxy on Fire 2, The Conduit
 * Provides comprehensive Android runtime environment emulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vitasdk.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include "fios.h"
#include "config.h"

// External debug function
extern void debugPrintf(const char *fmt, ...);

// Android system state
static int android_initialized = 0;
static void *android_vm = NULL;
static void *android_native_activity = NULL;
static void *android_asset_manager = NULL;

// Android display metrics
static struct {
    int width;
    int height;
    float density;
    int orientation;
} android_display = {960, 544, 1.0f, 1}; // Landscape

// Android input state
static struct {
    int touch_enabled;
    int gyro_enabled;
    int accelerometer_enabled;
} android_input = {1, 0, 0};

// Android audio state
static struct {
    int sample_rate;
    int buffer_size;
    int channels;
} android_audio = {44100, 1024, 2};

// ===== ANDROID SYSTEM INITIALIZATION =====

void android_api_init(void) {
    if (android_initialized) {
        debugPrintf("Android API: Already initialized\n");
        return;
    }

    debugPrintf("Android API: Starting comprehensive initialization...\n");

    // Initialize Android VM
    android_vm = malloc(sizeof(int));
    *(int*)android_vm = 0x12345678;
    debugPrintf("Android API: VM initialized at %p\n", android_vm);

    // Initialize Native Activity
    android_native_activity = malloc(256);
    memset(android_native_activity, 0, 256);
    debugPrintf("Android API: Native Activity initialized at %p\n", android_native_activity);

    // Initialize Asset Manager
    android_asset_manager = malloc(sizeof(int));
    *(int*)android_asset_manager = 0x87654321;
    debugPrintf("Android API: Asset Manager initialized at %p\n", android_asset_manager);

    // Set up display configuration
    android_display.width = 960;
    android_display.height = 544;
    android_display.density = 1.0f;
    android_display.orientation = 1; // Landscape

    // Configure audio system
    android_audio.sample_rate = 44100;
    android_audio.buffer_size = 1024;
    android_audio.channels = 2;

    // Configure input system
    android_input.touch_enabled = config_get_touch_controls();
    android_input.gyro_enabled = config_get_gyroscope();
    android_input.accelerometer_enabled = 0;

    android_initialized = 1;
    debugPrintf("Android API: Comprehensive initialization complete\n");
}

// ===== ANDROID CONTEXT API =====

void *android_getApplicationContext(void) {
    debugPrintf("Android: getApplicationContext() called\n");
    return android_native_activity;
}

void *android_getSystemService(void *context, const char *service) {
    debugPrintf("Android: getSystemService(%s) called\n", service ? service : "NULL");

    if (!service) return NULL;

    if (strcmp(service, "window") == 0) {
        static int window_manager = 0x71717171;
        return &window_manager;
    } else if (strcmp(service, "audio") == 0) {
        static int audio_manager = 0x72727272;
        return &audio_manager;
    } else if (strcmp(service, "connectivity") == 0) {
        static int connectivity_manager = 0x73737373;
        return &connectivity_manager;
    } else if (strcmp(service, "sensor") == 0) {
        static int sensor_manager = 0x74747474;
        return &sensor_manager;
    } else if (strcmp(service, "input_method") == 0) {
        static int input_method_manager = 0x75757575;
        return &input_method_manager;
    } else if (strcmp(service, "power") == 0) {
        static int power_manager = 0x76767676;
        return &power_manager;
    }

    debugPrintf("Android: Unknown service requested: %s\n", service);
    return NULL;
}

void *android_getAssets(void *context) {
    debugPrintf("Android: getAssets() called\n");
    return android_asset_manager;
}

void *android_getPackageName(void *context) {
    debugPrintf("Android: getPackageName() called\n");
    return "com.hotdog.fluffydiver";
}

void *android_getExternalFilesDir(void *context, void *type) {
    debugPrintf("Android: getExternalFilesDir() called\n");
    return "ux0:data/fluffydiver/external/";
}

void *android_getFilesDir(void *context) {
    debugPrintf("Android: getFilesDir() called\n");
    return "ux0:data/fluffydiver/files/";
}

void *android_getCacheDir(void *context) {
    debugPrintf("Android: getCacheDir() called\n");
    return "ux0:data/fluffydiver/cache/";
}

int android_getVersionCode(void *context) {
    debugPrintf("Android: getVersionCode() called\n");
    return 1;
}

void *android_getVersionName(void *context) {
    debugPrintf("Android: getVersionName() called\n");
    return "1.0";
}

// ===== ANDROID ASSET MANAGER API =====

void *android_asset_open(void *asset_manager, const char *filename, int mode) {
    debugPrintf("Android: AssetManager.open(%s, %d) called\n", filename ? filename : "NULL", mode);

    if (!filename) return NULL;

    // Use FIOS for asset access
    return fios_asset_open(filename, "rb");
}

int android_asset_read(void *asset, void *buffer, int size) {
    debugPrintf("Android: Asset.read(buffer=%p, size=%d) called\n", buffer, size);

    if (!asset || !buffer || size <= 0) return 0;

    return fread(buffer, 1, size, (FILE*)asset);
}

int android_asset_seek(void *asset, int offset, int whence) {
    debugPrintf("Android: Asset.seek(offset=%d, whence=%d) called\n", offset, whence);

    if (!asset) return -1;

    return fseek((FILE*)asset, offset, whence);
}

int android_asset_getLength(void *asset) {
    debugPrintf("Android: Asset.getLength() called\n");

    if (!asset) return 0;

    FILE *file = (FILE*)asset;
    long current = ftell(file);
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, current, SEEK_SET);

    return (int)length;
}

void android_asset_close(void *asset) {
    debugPrintf("Android: Asset.close() called\n");

    if (asset) {
        fclose((FILE*)asset);
    }
}

int android_asset_getRemainingLength(void *asset) {
    debugPrintf("Android: Asset.getRemainingLength() called\n");

    if (!asset) return 0;

    FILE *file = (FILE*)asset;
    long current = ftell(file);
    fseek(file, 0, SEEK_END);
    long total = ftell(file);
    fseek(file, current, SEEK_SET);

    return (int)(total - current);
}

// ===== ANDROID DISPLAY API =====

void android_getDisplayMetrics(void *context, int *width, int *height, float *density) {
    debugPrintf("Android: getDisplayMetrics() called\n");

    if (width) *width = android_display.width;
    if (height) *height = android_display.height;
    if (density) *density = android_display.density;

    debugPrintf("Android: Display metrics: %dx%d, density=%.1f\n",
                android_display.width, android_display.height, android_display.density);
}

int android_getOrientation(void *context) {
    debugPrintf("Android: getOrientation() called -> %d\n", android_display.orientation);
    return android_display.orientation;
}

int android_getScreenWidth(void *context) {
    debugPrintf("Android: getScreenWidth() -> %d\n", android_display.width);
    return android_display.width;
}

int android_getScreenHeight(void *context) {
    debugPrintf("Android: getScreenHeight() -> %d\n", android_display.height);
    return android_display.height;
}

float android_getScreenDensity(void *context) {
    debugPrintf("Android: getScreenDensity() -> %.1f\n", android_display.density);
    return android_display.density;
}

// ===== ANDROID AUDIO API =====

int android_audio_getSampleRate(void *audio_manager) {
    debugPrintf("Android: AudioManager.getSampleRate() -> %d\n", android_audio.sample_rate);
    return android_audio.sample_rate;
}

int android_audio_getFramesPerBuffer(void *audio_manager) {
    debugPrintf("Android: AudioManager.getFramesPerBuffer() -> %d\n", android_audio.buffer_size);
    return android_audio.buffer_size;
}

int android_audio_getChannelCount(void *audio_manager) {
    debugPrintf("Android: AudioManager.getChannelCount() -> %d\n", android_audio.channels);
    return android_audio.channels;
}

void android_audio_setVolume(void *audio_manager, float volume) {
    debugPrintf("Android: AudioManager.setVolume(%.2f) called\n", volume);
    // Could integrate with Vita audio system
}

float android_audio_getVolume(void *audio_manager) {
    float volume = config_get_master_volume() / 100.0f;
    debugPrintf("Android: AudioManager.getVolume() -> %.2f\n", volume);
    return volume;
}

// ===== ANDROID INPUT API =====

int android_input_isTouchEnabled(void *context) {
    debugPrintf("Android: isTouchEnabled() -> %d\n", android_input.touch_enabled);
    return android_input.touch_enabled;
}

int android_input_isGyroscopeEnabled(void *context) {
    debugPrintf("Android: isGyroscopeEnabled() -> %d\n", android_input.gyro_enabled);
    return android_input.gyro_enabled;
}

int android_input_isAccelerometerEnabled(void *context) {
    debugPrintf("Android: isAccelerometerEnabled() -> %d\n", android_input.accelerometer_enabled);
    return android_input.accelerometer_enabled;
}

void android_input_setTouchEnabled(void *context, int enabled) {
    debugPrintf("Android: setTouchEnabled(%d) called\n", enabled);
    android_input.touch_enabled = enabled;
}

void android_input_setGyroscopeEnabled(void *context, int enabled) {
    debugPrintf("Android: setGyroscopeEnabled(%d) called\n", enabled);
    android_input.gyro_enabled = enabled;
}

// ===== ANDROID SENSOR API =====

typedef struct {
    float x, y, z;
    uint64_t timestamp;
} AndroidSensorEvent;

void android_sensor_getAccelerometer(AndroidSensorEvent *event) {
    debugPrintf("Android: getAccelerometer() called\n");

    if (event) {
        // Could integrate with Vita motion sensor
        event->x = 0.0f;
        event->y = -9.8f; // Gravity
        event->z = 0.0f;

        struct timeval tv;
        gettimeofday(&tv, NULL);
        event->timestamp = tv.tv_sec * 1000000LL + tv.tv_usec;
    }
}

void android_sensor_getGyroscope(AndroidSensorEvent *event) {
    debugPrintf("Android: getGyroscope() called\n");

    if (event) {
        // Could integrate with Vita gyroscope
        event->x = 0.0f;
        event->y = 0.0f;
        event->z = 0.0f;

        struct timeval tv;
        gettimeofday(&tv, NULL);
        event->timestamp = tv.tv_sec * 1000000LL + tv.tv_usec;
    }
}

// ===== ANDROID NETWORK API =====

int android_network_isConnected(void *connectivity_manager) {
    debugPrintf("Android: ConnectivityManager.isConnected() -> 0 (no network)\n");
    return 0; // Vita has no standard network for games
}

int android_network_isWifiConnected(void *connectivity_manager) {
    debugPrintf("Android: ConnectivityManager.isWifiConnected() -> 0\n");
    return 0;
}

int android_network_isMobileConnected(void *connectivity_manager) {
    debugPrintf("Android: ConnectivityManager.isMobileConnected() -> 0\n");
    return 0;
}

// ===== ANDROID STORAGE API =====

long long android_storage_getFreeSpace(void *context) {
    debugPrintf("Android: getFreeSpace() called\n");

    // Use FIOS to get actual free space
    long long free_space = fios_get_free_space("ux0:");
    if (free_space < 0) {
        free_space = 1024LL * 1024LL * 1024LL; // 1GB fallback
    }

    debugPrintf("Android: Free space: %lld bytes\n", free_space);
    return free_space;
}

long long android_storage_getTotalSpace(void *context) {
    debugPrintf("Android: getTotalSpace() called\n");

    // Estimate total space
    long long total_space = 4LL * 1024LL * 1024LL * 1024LL; // 4GB estimate
    debugPrintf("Android: Total space: %lld bytes\n", total_space);
    return total_space;
}

// ===== ANDROID PREFERENCES API =====

static struct {
    char keys[32][64];
    char values[32][256];
    int count;
} android_prefs = {{0}, {0}, 0};

void *android_getSharedPreferences(void *context, const char *name, int mode) {
    debugPrintf("Android: getSharedPreferences(%s, %d) called\n", name ? name : "NULL", mode);
    static int fake_prefs = 0x80808080;
    return &fake_prefs;
}

int android_prefs_getInt(void *prefs, const char *key, int default_value) {
    debugPrintf("Android: SharedPreferences.getInt(%s, %d) called\n", key ? key : "NULL", default_value);

    if (!key) return default_value;

    // Search for existing key
    for (int i = 0; i < android_prefs.count; i++) {
        if (strcmp(android_prefs.keys[i], key) == 0) {
            int value = atoi(android_prefs.values[i]);
            debugPrintf("Android: Found stored value: %d\n", value);
            return value;
        }
    }

    debugPrintf("Android: Using default value: %d\n", default_value);
    return default_value;
}

void android_prefs_putInt(void *prefs, const char *key, int value) {
    debugPrintf("Android: SharedPreferences.putInt(%s, %d) called\n", key ? key : "NULL", value);

    if (!key || android_prefs.count >= 32) return;

    // Update existing or add new
    for (int i = 0; i < android_prefs.count; i++) {
        if (strcmp(android_prefs.keys[i], key) == 0) {
            snprintf(android_prefs.values[i], sizeof(android_prefs.values[i]), "%d", value);
            return;
        }
    }

    // Add new entry
    strcpy(android_prefs.keys[android_prefs.count], key);
    snprintf(android_prefs.values[android_prefs.count], sizeof(android_prefs.values[android_prefs.count]), "%d", value);
    android_prefs.count++;
}

void *android_prefs_getString(void *prefs, const char *key, const char *default_value) {
    debugPrintf("Android: SharedPreferences.getString(%s, %s) called\n",
                key ? key : "NULL", default_value ? default_value : "NULL");

    if (!key) return (void*)default_value;

    // Search for existing key
    for (int i = 0; i < android_prefs.count; i++) {
        if (strcmp(android_prefs.keys[i], key) == 0) {
            debugPrintf("Android: Found stored string: %s\n", android_prefs.values[i]);
            return android_prefs.values[i];
        }
    }

    debugPrintf("Android: Using default string: %s\n", default_value ? default_value : "NULL");
    return (void*)default_value;
}

void android_prefs_putString(void *prefs, const char *key, const char *value) {
    debugPrintf("Android: SharedPreferences.putString(%s, %s) called\n",
                key ? key : "NULL", value ? value : "NULL");

    if (!key || !value || android_prefs.count >= 32) return;

    // Update existing or add new
    for (int i = 0; i < android_prefs.count; i++) {
        if (strcmp(android_prefs.keys[i], key) == 0) {
            strncpy(android_prefs.values[i], value, sizeof(android_prefs.values[i]) - 1);
            android_prefs.values[i][sizeof(android_prefs.values[i]) - 1] = '\0';
            return;
        }
    }

    // Add new entry
    strcpy(android_prefs.keys[android_prefs.count], key);
    strncpy(android_prefs.values[android_prefs.count], value, sizeof(android_prefs.values[android_prefs.count]) - 1);
    android_prefs.values[android_prefs.count][sizeof(android_prefs.values[android_prefs.count]) - 1] = '\0';
    android_prefs.count++;
}

// ===== ANDROID LIFECYCLE API =====

void android_onCreate(void *activity) {
    debugPrintf("Android: onCreate() called\n");
    android_api_init();
}

void android_onStart(void *activity) {
    debugPrintf("Android: onStart() called\n");
}

void android_onResume(void *activity) {
    debugPrintf("Android: onResume() called\n");
}

void android_onPause(void *activity) {
    debugPrintf("Android: onPause() called\n");
}

void android_onStop(void *activity) {
    debugPrintf("Android: onStop() called\n");
}

void android_onDestroy(void *activity) {
    debugPrintf("Android: onDestroy() called\n");
}

// ===== ANDROID UTILITY API =====

void android_vibrate(void *context, int milliseconds) {
    debugPrintf("Android: vibrate(%d) called\n", milliseconds);
    // Could integrate with Vita vibration if available
}

void android_startActivity(void *context, const char *action, const char *data) {
    debugPrintf("Android: startActivity(%s, %s) called\n",
                action ? action : "NULL", data ? data : "NULL");
    // Cannot start activities on Vita
}

int android_log_print(int priority, const char *tag, const char *message) {
    debugPrintf("Android Log [%s]: %s\n", tag ? tag : "NULL", message ? message : "NULL");
    return 0;
}

void android_runOnUiThread(void *context, void *runnable) {
    debugPrintf("Android: runOnUiThread() called\n");
    // Execute immediately since we don't have separate UI thread
    if (runnable) {
        void (*func)(void) = (void(*)(void))runnable;
        func();
    }
}

// ===== ANDROID BITMAP API =====

void *android_bitmap_createBitmap(int width, int height, int config) {
    debugPrintf("Android: Bitmap.createBitmap(%d, %d, %d) called\n", width, height, config);

    int bytes_per_pixel = 4; // RGBA
    int size = width * height * bytes_per_pixel;
    void *bitmap_data = malloc(size + 16); // Extra space for metadata

    if (bitmap_data) {
        memset(bitmap_data, 0, size + 16);
        *(int*)bitmap_data = width;
        *(int*)((char*)bitmap_data + 4) = height;
        *(int*)((char*)bitmap_data + 8) = config;
        *(int*)((char*)bitmap_data + 12) = size;

        debugPrintf("Android: Created bitmap %dx%d at %p (size: %d)\n", width, height, bitmap_data, size);
    }

    return bitmap_data;
}

void android_bitmap_recycle(void *bitmap) {
    debugPrintf("Android: Bitmap.recycle() called\n");

    if (bitmap) {
        free(bitmap);
    }
}

int android_bitmap_getWidth(void *bitmap) {
    if (!bitmap) return 0;
    return *(int*)bitmap;
}

int android_bitmap_getHeight(void *bitmap) {
    if (!bitmap) return 0;
    return *(int*)((char*)bitmap + 4);
}

// ===== ANDROID API CLEANUP =====

void android_api_cleanup(void) {
    debugPrintf("Android API: Cleaning up...\n");

    if (android_vm) {
        free(android_vm);
        android_vm = NULL;
    }

    if (android_native_activity) {
        free(android_native_activity);
        android_native_activity = NULL;
    }

    if (android_asset_manager) {
        free(android_asset_manager);
        android_asset_manager = NULL;
    }

    android_initialized = 0;
    debugPrintf("Android API: Cleanup complete\n");
}
