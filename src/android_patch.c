/*
 * android_patch.c - Complete Android API Bridge for Fluffy Diver
 * Based on GTA SA Vita + analysis of successful ports
 * Combines original android_patch.c with comprehensive environment emulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/motion.h>
#include <psp2/touch.h>
#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <sys/time.h>

#include "config.h"
#include "fios.h"
#include "jni_patch.h"  // Include JNI types

// External debug function
extern void debugPrintf(const char *fmt, ...);

// ===== ANDROID LOG PRIORITIES =====
#define ANDROID_LOG_UNKNOWN 0
#define ANDROID_LOG_DEFAULT 1
#define ANDROID_LOG_VERBOSE 2
#define ANDROID_LOG_DEBUG   3
#define ANDROID_LOG_INFO    4
#define ANDROID_LOG_WARN    5
#define ANDROID_LOG_ERROR   6
#define ANDROID_LOG_FATAL   7
#define ANDROID_LOG_SILENT  8

// ===== ANDROID SENSOR TYPES =====
#define ASENSOR_TYPE_ACCELEROMETER 1
#define ASENSOR_TYPE_MAGNETIC_FIELD 2
#define ASENSOR_TYPE_GYROSCOPE 4
#define ASENSOR_TYPE_LIGHT 5
#define ASENSOR_TYPE_PROXIMITY 8

// ===== ANDROID SENSOR STRUCTURES =====
typedef struct ASensorVector {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
        struct {
            float azimuth;
            float pitch;
            float roll;
        };
    };
    int8_t status;
    uint8_t reserved[3];
} ASensorVector;

typedef struct ASensorEvent {
    int32_t version;
    int32_t sensor;
    int32_t type;
    int32_t reserved0;
    int64_t timestamp;
    union {
        float data[16];
        ASensorVector vector;
        ASensorVector acceleration;
        ASensorVector magnetic;
        ASensorVector gyro;
        float temperature;
        float distance;
        float light;
        float pressure;
    };
    int32_t reserved1[4];
} ASensorEvent;

// ===== ANDROID LOGGING FUNCTIONS =====

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const char *prio_str;
    switch (prio) {
        case ANDROID_LOG_VERBOSE: prio_str = "V"; break;
        case ANDROID_LOG_DEBUG:   prio_str = "D"; break;
        case ANDROID_LOG_INFO:    prio_str = "I"; break;
        case ANDROID_LOG_WARN:    prio_str = "W"; break;
        case ANDROID_LOG_ERROR:   prio_str = "E"; break;
        case ANDROID_LOG_FATAL:   prio_str = "F"; break;
        default:                  prio_str = "?"; break;
    }

    debugPrintf("Android[%s/%s]: ", prio_str, tag ? tag : "NULL");
    vprintf(fmt, args);
    debugPrintf("\n");

    va_end(args);
    return 0;
}

int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args) {
    const char *prio_str;
    switch (prio) {
        case ANDROID_LOG_VERBOSE: prio_str = "V"; break;
        case ANDROID_LOG_DEBUG:   prio_str = "D"; break;
        case ANDROID_LOG_INFO:    prio_str = "I"; break;
        case ANDROID_LOG_WARN:    prio_str = "W"; break;
        case ANDROID_LOG_ERROR:   prio_str = "E"; break;
        case ANDROID_LOG_FATAL:   prio_str = "F"; break;
        default:                  prio_str = "?"; break;
    }

    debugPrintf("Android[%s/%s]: ", prio_str, tag ? tag : "NULL");
    vprintf(fmt, args);
    debugPrintf("\n");

    return 0;
}

int __android_log_write(int prio, const char *tag, const char *text) {
    return __android_log_print(prio, tag, "%s", text);
}

// ===== ANDROID ASSET MANAGER IMPLEMENTATION =====

typedef struct AAssetManager AAssetManager;
typedef struct AAsset AAsset;

// Asset manager using FIOS
AAsset *AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {
    debugPrintf("Android: AAssetManager_open(%s, %d)\n", filename ? filename : "NULL", mode);

    if (!filename) return NULL;

    // Use FIOS asset system
    FILE *file = fios_asset_open(filename, "rb");
    if (file) {
        debugPrintf("Android: Successfully opened asset: %s\n", filename);
        return (AAsset *)file;
    }

    debugPrintf("Android: Failed to open asset: %s\n", filename);
    return NULL;
}

int AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset || !buf) return 0;

    int bytes_read = fread(buf, 1, count, (FILE *)asset);
    debugPrintf("Android: AAsset_read(%p, %zu) -> %d bytes\n", buf, count, bytes_read);
    return bytes_read;
}

off_t AAsset_seek(AAsset *asset, off_t offset, int whence) {
    if (!asset) return -1;

    int result = fseek((FILE *)asset, offset, whence);
    debugPrintf("Android: AAsset_seek(%ld, %d) -> %d\n", offset, whence, result);
    return result;
}

off_t AAsset_getLength(AAsset *asset) {
    if (!asset) return 0;

    FILE *file = (FILE *)asset;
    long current = ftell(file);
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, current, SEEK_SET);

    debugPrintf("Android: AAsset_getLength() -> %ld\n", length);
    return length;
}

off_t AAsset_getRemainingLength(AAsset *asset) {
    if (!asset) return 0;

    FILE *file = (FILE *)asset;
    long current = ftell(file);
    fseek(file, 0, SEEK_END);
    long total = ftell(file);
    fseek(file, current, SEEK_SET);

    long remaining = total - current;
    debugPrintf("Android: AAsset_getRemainingLength() -> %ld\n", remaining);
    return remaining;
}

void AAsset_close(AAsset *asset) {
    debugPrintf("Android: AAsset_close()\n");
    if (asset) {
        fclose((FILE *)asset);
    }
}

// ===== ANDROID CONFIGURATION STUBS =====

typedef struct AConfiguration AConfiguration;

AConfiguration *AConfiguration_new() {
    debugPrintf("Android: AConfiguration_new()\n");
    return (AConfiguration *)malloc(sizeof(int));
}

void AConfiguration_delete(AConfiguration *config) {
    debugPrintf("Android: AConfiguration_delete()\n");
    if (config) free(config);
}

void AConfiguration_fromAssetManager(AConfiguration *out, AAssetManager *am) {
    debugPrintf("Android: AConfiguration_fromAssetManager()\n");
}

int32_t AConfiguration_getOrientation(AConfiguration *config) {
    debugPrintf("Android: AConfiguration_getOrientation() -> 2 (landscape)\n");
    return 2; // ACONFIGURATION_ORIENTATION_LAND
}

int32_t AConfiguration_getDensity(AConfiguration *config) {
    debugPrintf("Android: AConfiguration_getDensity() -> 160 (medium)\n");
    return 160; // ACONFIGURATION_DENSITY_MEDIUM
}

int32_t AConfiguration_getScreenSize(AConfiguration *config) {
    debugPrintf("Android: AConfiguration_getScreenSize() -> 2 (normal)\n");
    return 2; // ACONFIGURATION_SCREENSIZE_NORMAL
}

int32_t AConfiguration_getScreenLong(AConfiguration *config) {
    debugPrintf("Android: AConfiguration_getScreenLong() -> 2 (long)\n");
    return 2; // ACONFIGURATION_SCREENLONG_LONG (for 16:9 aspect ratio)
}

// ===== ANDROID NATIVE ACTIVITY STUBS =====

typedef struct {
    void *callbacks;
    void *vm;
    void *env;
    void *clazz;
    const char *internalDataPath;
    const char *externalDataPath;
    int32_t sdkVersion;
    void *instance;
    void *assetManager;
    const char *obbPath;
} ANativeActivity;

// Enhanced fake native activity with proper paths
static ANativeActivity fake_native_activity = {
    NULL,                              // callbacks
    NULL,                              // vm
    NULL,                              // env
    NULL,                              // clazz
    "ux0:data/fluffydiver/internal/",  // internalDataPath
    "ux0:data/fluffydiver/external/",  // externalDataPath
    23,                                // sdkVersion (Android 6.0)
    NULL,                              // instance
    NULL,                              // assetManager
    "ux0:data/fluffydiver/obb/"        // obbPath
};

ANativeActivity *android_get_activity() {
    debugPrintf("Android: android_get_activity() -> %p\n", &fake_native_activity);
    return &fake_native_activity;
}

void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
    debugPrintf("Android: ANativeActivity_onCreate(activity=%p, savedState=%p, size=%zu)\n",
                activity, savedState, savedStateSize);
}

void ANativeActivity_onDestroy(ANativeActivity *activity) {
    debugPrintf("Android: ANativeActivity_onDestroy(activity=%p)\n", activity);
}

void ANativeActivity_onStart(ANativeActivity *activity) {
    debugPrintf("Android: ANativeActivity_onStart(activity=%p)\n", activity);
}

void ANativeActivity_onResume(ANativeActivity *activity) {
    debugPrintf("Android: ANativeActivity_onResume(activity=%p)\n", activity);
}

void ANativeActivity_onPause(ANativeActivity *activity) {
    debugPrintf("Android: ANativeActivity_onPause(activity=%p)\n", activity);
}

void ANativeActivity_onStop(ANativeActivity *activity) {
    debugPrintf("Android: ANativeActivity_onStop(activity=%p)\n", activity);
}

// ===== ANDROID INPUT EVENT STUBS =====

typedef struct AInputQueue AInputQueue;
typedef struct AInputEvent AInputEvent;

int32_t AInputQueue_getEvent(AInputQueue *queue, AInputEvent **outEvent) {
    debugPrintf("Android: AInputQueue_getEvent()\n");
    return -1; // No events available
}

int32_t AInputQueue_preDispatchEvent(AInputQueue *queue, AInputEvent *event) {
    debugPrintf("Android: AInputQueue_preDispatchEvent()\n");
    return 0;
}

void AInputQueue_finishEvent(AInputQueue *queue, AInputEvent *event, int handled) {
    debugPrintf("Android: AInputQueue_finishEvent(handled=%d)\n", handled);
}

// Input event properties
int32_t AInputEvent_getType(const AInputEvent *event) {
    debugPrintf("Android: AInputEvent_getType()\n");
    return 1; // AINPUT_EVENT_TYPE_KEY
}

int32_t AInputEvent_getDeviceId(const AInputEvent *event) {
    debugPrintf("Android: AInputEvent_getDeviceId()\n");
    return 0;
}

int32_t AInputEvent_getSource(const AInputEvent *event) {
    debugPrintf("Android: AInputEvent_getSource()\n");
    return 0x00000101; // AINPUT_SOURCE_KEYBOARD
}

// ===== ANDROID NATIVE WINDOW STUBS =====

typedef struct ANativeWindow ANativeWindow;

int32_t ANativeWindow_getWidth(ANativeWindow *window) {
    debugPrintf("Android: ANativeWindow_getWidth() -> 960\n");
    return 960;
}

int32_t ANativeWindow_getHeight(ANativeWindow *window) {
    debugPrintf("Android: ANativeWindow_getHeight() -> 544\n");
    return 544;
}

int32_t ANativeWindow_getFormat(ANativeWindow *window) {
    debugPrintf("Android: ANativeWindow_getFormat() -> 4 (RGB_565)\n");
    return 4; // WINDOW_FORMAT_RGB_565
}

int32_t ANativeWindow_setBuffersGeometry(ANativeWindow *window, int32_t width, int32_t height, int32_t format) {
    debugPrintf("Android: ANativeWindow_setBuffersGeometry(%d, %d, %d)\n", width, height, format);
    return 0;
}

// ===== ANDROID SENSOR MANAGER STUBS =====

typedef struct ASensorManager ASensorManager;
typedef struct ASensor ASensor;
typedef struct ASensorEventQueue ASensorEventQueue;

ASensorManager *ASensorManager_getInstance() {
    debugPrintf("Android: ASensorManager_getInstance()\n");
    static int fake_sensor_manager = 0x12345678;
    return (ASensorManager *)&fake_sensor_manager;
}

ASensor const *ASensorManager_getDefaultSensor(ASensorManager *manager, int type) {
    debugPrintf("Android: ASensorManager_getDefaultSensor(type=%d)\n", type);
    static int fake_sensor = 0x87654321;
    return (ASensor *)&fake_sensor;
}

ASensorEventQueue *ASensorManager_createEventQueue(ASensorManager *manager, void *looper, int ident, void *callback, void *data) {
    debugPrintf("Android: ASensorManager_createEventQueue()\n");
    static int fake_queue = 0x11111111;
    return (ASensorEventQueue *)&fake_queue;
}

int ASensorEventQueue_enableSensor(ASensorEventQueue *queue, ASensor const *sensor) {
    debugPrintf("Android: ASensorEventQueue_enableSensor()\n");
    return 0;
}

int ASensorEventQueue_disableSensor(ASensorEventQueue *queue, ASensor const *sensor) {
    debugPrintf("Android: ASensorEventQueue_disableSensor()\n");
    return 0;
}

ssize_t ASensorEventQueue_getEvents(ASensorEventQueue *queue, ASensorEvent *events, size_t count) {
    debugPrintf("Android: ASensorEventQueue_getEvents(count=%zu)\n", count);

    // Simulate sensor data if gyroscope is enabled
    if (config_get_gyroscope() && count > 0) {
        // Could read actual Vita motion sensor here
        events[0].type = ASENSOR_TYPE_GYROSCOPE;
        events[0].gyro.x = 0.0f;
        events[0].gyro.y = 0.0f;
        events[0].gyro.z = 0.0f;

        SceRtcTick tick;
        sceRtcGetCurrentTick(&tick);
        events[0].timestamp = tick.tick;

        return 1;
    }

    return 0; // No events
}

// ===== ANDROID SYSTEM PROPERTY STUBS =====

int __system_property_get(const char *name, char *value) {
    debugPrintf("Android: __system_property_get(%s)\n", name ? name : "NULL");

    if (!name || !value) {
        if (value) value[0] = '\0';
        return 0;
    }

    if (strcmp(name, "ro.build.version.sdk") == 0) {
        strcpy(value, "23");
        return 2;
    } else if (strcmp(name, "ro.product.model") == 0) {
        strcpy(value, "PS Vita");
        return 7;
    } else if (strcmp(name, "ro.product.manufacturer") == 0) {
        strcpy(value, "Sony");
        return 4;
    } else if (strcmp(name, "ro.product.device") == 0) {
        strcpy(value, "vita");
        return 4;
    } else if (strcmp(name, "ro.build.version.release") == 0) {
        strcpy(value, "6.0");
        return 3;
    } else if (strcmp(name, "ro.build.version.codename") == 0) {
        strcpy(value, "REL");
        return 3;
    } else if (strcmp(name, "ro.product.cpu.abi") == 0) {
        strcpy(value, "armeabi-v7a");
        return 11;
    } else if (strcmp(name, "ro.hardware") == 0) {
        strcpy(value, "vita");
        return 4;
    }

    value[0] = '\0';
    return 0;
}

// ===== ANDROID LOOPER STUBS =====

typedef struct ALooper ALooper;

ALooper *ALooper_forThread() {
    debugPrintf("Android: ALooper_forThread()\n");
    static int fake_looper = 0x44444444;
    return (ALooper *)&fake_looper;
}

ALooper *ALooper_prepare(int opts) {
    debugPrintf("Android: ALooper_prepare(opts=%d)\n", opts);
    static int fake_looper = 0x55555555;
    return (ALooper *)&fake_looper;
}

int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    debugPrintf("Android: ALooper_pollOnce(timeout=%d)\n", timeoutMillis);

    // Simulate some polling delay
    if (timeoutMillis > 0) {
        sceKernelDelayThread(timeoutMillis * 1000); // Convert ms to us
    }

    return -1; // ALOOPER_POLL_TIMEOUT
}

int ALooper_pollAll(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    debugPrintf("Android: ALooper_pollAll(timeout=%d)\n", timeoutMillis);
    return ALooper_pollOnce(timeoutMillis, outFd, outEvents, outData);
}

// ===== ANDROID STORAGE MANAGER STUBS =====

typedef struct AStorageManager AStorageManager;

long long AStorageManager_getCacheQuotaBytes(AStorageManager *mgr, const char *uuid) {
    debugPrintf("Android: AStorageManager_getCacheQuotaBytes()\n");
    return 100 * 1024 * 1024; // 100MB
}

long long AStorageManager_getCacheSizeBytes(AStorageManager *mgr, const char *uuid) {
    debugPrintf("Android: AStorageManager_getCacheSizeBytes()\n");
    return 10 * 1024 * 1024; // 10MB
}

// ===== ANDROID CONTENT RESOLVER STUBS =====

typedef struct AContentResolver AContentResolver;

void *AContentResolver_query(AContentResolver *resolver, const char *uri, const char **projection,
                             const char *selection, const char **selectionArgs, const char *sortOrder) {
    debugPrintf("Android: AContentResolver_query(uri=%s)\n", uri ? uri : "NULL");
    return NULL; // No content provider
                             }

                             // ===== ANDROID MISC FUNCTIONS =====

                             // android_set_abort_message is defined in default_dynlib.c

                             void *android_dlopen(const char *filename, int flag) {
                                 debugPrintf("Android: android_dlopen(%s, %d)\n", filename ? filename : "NULL", flag);
                                 return NULL; // Dynamic loading not supported
                             }

                             void *android_dlsym(void *handle, const char *symbol) {
                                 debugPrintf("Android: android_dlsym(handle=%p, symbol=%s)\n", handle, symbol ? symbol : "NULL");
                                 return NULL;
                             }

                             int android_dlclose(void *handle) {
                                 debugPrintf("Android: android_dlclose(handle=%p)\n", handle);
                                 return 0;
                             }

                             char *android_dlerror(void) {
                                 debugPrintf("Android: android_dlerror()\n");
                                 return "Dynamic loading not supported on PS Vita";
                             }

                             // ===== ANDROID JAVA VM STUBS =====

                             // JavaVM is already defined in jni_patch.h

                             int JNI_GetDefaultJavaVMInitArgs(void *args) {
                                 debugPrintf("Android: JNI_GetDefaultJavaVMInitArgs()\n");
                                 return 0;
                             }

                             int JNI_CreateJavaVM(JavaVM **pvm, JNIEnv **penv, void *args) {
                                 debugPrintf("Android: JNI_CreateJavaVM()\n");

                                 // Use fake VM and environment
                                 extern void *fake_env;
                                 if (pvm) *pvm = (JavaVM *)fake_env;
                                 if (penv) *penv = (JNIEnv *)fake_env;

                                 return 0;
                             }

                             int JNI_GetCreatedJavaVMs(JavaVM **vmBuf, jsize bufLen, jsize *nVMs) {
                                 debugPrintf("Android: JNI_GetCreatedJavaVMs()\n");

                                 if (nVMs) *nVMs = 0;
                                 return 0;
                             }

                             // ===== ANDROID TIME ZONE STUBS =====

                             void *android_timezone_get_tz_id(void) {
                                 debugPrintf("Android: android_timezone_get_tz_id()\n");
                                 return "UTC";
                             }

                             void *android_timezone_get_tz_name(void) {
                                 debugPrintf("Android: android_timezone_get_tz_name()\n");
                                 return "Coordinated Universal Time";
                             }

                             // ===== ANDROID NETWORK CONNECTIVITY STUBS =====

                             int android_network_get_type(void) {
                                 debugPrintf("Android: android_network_get_type() -> 0 (none)\n");
                                 return 0; // TYPE_NONE
                             }

                             int android_network_is_connected(void) {
                                 debugPrintf("Android: android_network_is_connected() -> 0\n");
                                 return 0; // Not connected
                             }

                             // ===== ANDROID MEDIA STUBS =====

                             typedef struct AMediaCodec AMediaCodec;
                             typedef struct AMediaFormat AMediaFormat;

                             AMediaCodec *AMediaCodec_createDecoderByType(const char *mime_type) {
                                 debugPrintf("Android: AMediaCodec_createDecoderByType(%s)\n", mime_type ? mime_type : "NULL");
                                 return NULL; // No media codec support
                             }

                             AMediaFormat *AMediaFormat_new() {
                                 debugPrintf("Android: AMediaFormat_new()\n");
                                 return NULL;
                             }

                             void AMediaFormat_delete(AMediaFormat *format) {
                                 debugPrintf("Android: AMediaFormat_delete()\n");
                             }

                             // ===== ANDROID CAMERA STUBS =====

                             typedef struct ACameraManager ACameraManager;

                             ACameraManager *ACameraManager_create() {
                                 debugPrintf("Android: ACameraManager_create()\n");
                                 return NULL; // No camera support
                             }

                             void ACameraManager_delete(ACameraManager *manager) {
                                 debugPrintf("Android: ACameraManager_delete()\n");
                             }

                             // ===== INITIALIZATION FUNCTION =====

                             void android_patch_init() {
                                 debugPrintf("Android: Android API bridge initialized (Enhanced GTA SA Vita + Successful Ports)\n");
                                 debugPrintf("Android: Comprehensive environment emulation enabled\n");
                                 debugPrintf("Android: FIOS integration active\n");
                                 debugPrintf("Android: Configuration system integration active\n");

                                 // Ensure required directories exist
                                 fios_mkdir("ux0:data/fluffydiver/internal");
                                 fios_mkdir("ux0:data/fluffydiver/external");
                                 fios_mkdir("ux0:data/fluffydiver/obb");
                                 fios_mkdir("ux0:data/fluffydiver/cache");

                                 debugPrintf("Android: Directory structure created\n");
                             }
