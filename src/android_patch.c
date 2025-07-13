/*
 * android_patch.c - Android API Bridge based on GTA SA Vita
 * Provides stub implementations for Android system calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/types.h>

// Android Log priorities (from android/log.h)
#define ANDROID_LOG_UNKNOWN 0
#define ANDROID_LOG_DEFAULT 1
#define ANDROID_LOG_VERBOSE 2
#define ANDROID_LOG_DEBUG   3
#define ANDROID_LOG_INFO    4
#define ANDROID_LOG_WARN    5
#define ANDROID_LOG_ERROR   6
#define ANDROID_LOG_FATAL   7
#define ANDROID_LOG_SILENT  8

// Android logging function
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

    printf("Android[%s/%s]: ", prio_str, tag ? tag : "NULL");
    vprintf(fmt, args);
    printf("\n");

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

    printf("Android[%s/%s]: ", prio_str, tag ? tag : "NULL");
    vprintf(fmt, args);
    printf("\n");

    return 0;
}

int __android_log_write(int prio, const char *tag, const char *text) {
    return __android_log_print(prio, tag, "%s", text);
}

// Android asset manager stubs
typedef struct AAssetManager AAssetManager;
typedef struct AAsset AAsset;

AAsset *AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {
    printf("Android: AAssetManager_open(%s, %d)\n", filename, mode);

    // Try to open the file from our asset directory
    char path[512];
    snprintf(path, sizeof(path), "ux0:data/fluffydiver/assets/%s", filename);

    FILE *file = fopen(path, "rb");
    if (file) {
        printf("Android: Successfully opened asset: %s\n", path);
        return (AAsset *)file;
    }

    printf("Android: Failed to open asset: %s\n", path);
    return NULL;
}

int AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset) return 0;
    return fread(buf, 1, count, (FILE *)asset);
}

off_t AAsset_seek(AAsset *asset, off_t offset, int whence) {
    if (!asset) return -1;
    return fseek((FILE *)asset, offset, whence);
}

off_t AAsset_getLength(AAsset *asset) {
    if (!asset) return 0;

    FILE *file = (FILE *)asset;
    long current = ftell(file);
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, current, SEEK_SET);

    return length;
}

void AAsset_close(AAsset *asset) {
    if (asset) {
        fclose((FILE *)asset);
    }
}

// Android configuration stubs
typedef struct AConfiguration AConfiguration;

AConfiguration *AConfiguration_new() {
    printf("Android: AConfiguration_new()\n");
    return (AConfiguration *)malloc(sizeof(int));
}

void AConfiguration_delete(AConfiguration *config) {
    printf("Android: AConfiguration_delete()\n");
    if (config) free(config);
}

void AConfiguration_fromAssetManager(AConfiguration *out, AAssetManager *am) {
    printf("Android: AConfiguration_fromAssetManager()\n");
}

int32_t AConfiguration_getOrientation(AConfiguration *config) {
    printf("Android: AConfiguration_getOrientation()\n");
    return 2; // ACONFIGURATION_ORIENTATION_LAND
}

int32_t AConfiguration_getDensity(AConfiguration *config) {
    printf("Android: AConfiguration_getDensity()\n");
    return 160; // ACONFIGURATION_DENSITY_MEDIUM
}

// Android native activity stubs
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

// Forward declarations
typedef struct AInputQueue AInputQueue;
typedef struct ANativeWindow ANativeWindow;

// Simplified fake native activity
static ANativeActivity fake_native_activity = {
    NULL,                         // callbacks
    NULL,                         // vm
    NULL,                         // env
    NULL,                         // clazz
    "ux0:data/fluffydiver/",      // internalDataPath
    "ux0:data/fluffydiver/",      // externalDataPath
    23,                           // sdkVersion
    NULL,                         // instance
    NULL,                         // assetManager
    "ux0:data/fluffydiver/"       // obbPath
};

ANativeActivity *android_get_activity() {
    return &fake_native_activity;
}

// Android input event stubs
typedef struct AInputEvent AInputEvent;

int32_t AInputQueue_getEvent(AInputQueue *queue, AInputEvent **outEvent) {
    printf("Android: AInputQueue_getEvent()\n");
    return -1; // No events
}

int32_t AInputQueue_preDispatchEvent(AInputQueue *queue, AInputEvent *event) {
    printf("Android: AInputQueue_preDispatchEvent()\n");
    return 0;
}

void AInputQueue_finishEvent(AInputQueue *queue, AInputEvent *event, int handled) {
    printf("Android: AInputQueue_finishEvent()\n");
}

// Android native window stubs
int32_t ANativeWindow_getWidth(ANativeWindow *window) {
    printf("Android: ANativeWindow_getWidth()\n");
    return 960;
}

int32_t ANativeWindow_getHeight(ANativeWindow *window) {
    printf("Android: ANativeWindow_getHeight()\n");
    return 544;
}

int32_t ANativeWindow_getFormat(ANativeWindow *window) {
    printf("Android: ANativeWindow_getFormat()\n");
    return 4; // WINDOW_FORMAT_RGB_565
}

// Android sensor stubs
typedef struct ASensorManager ASensorManager;
typedef struct ASensor ASensor;
typedef struct ASensorEventQueue ASensorEventQueue;

ASensorManager *ASensorManager_getInstance() {
    printf("Android: ASensorManager_getInstance()\n");
    static int fake_sensor_manager = 0x12345678;
    return (ASensorManager *)&fake_sensor_manager;
}

ASensor const *ASensorManager_getDefaultSensor(ASensorManager *manager, int type) {
    printf("Android: ASensorManager_getDefaultSensor(%d)\n", type);
    static int fake_sensor = 0x87654321;
    return (ASensor *)&fake_sensor;
}

ASensorEventQueue *ASensorManager_createEventQueue(ASensorManager *manager, void *looper, int ident, void *callback, void *data) {
    printf("Android: ASensorManager_createEventQueue()\n");
    static int fake_queue = 0x11111111;
    return (ASensorEventQueue *)&fake_queue;
}

// System property stubs
int __system_property_get(const char *name, char *value) {
    printf("Android: __system_property_get(%s)\n", name);

    if (strcmp(name, "ro.build.version.sdk") == 0) {
        strcpy(value, "23");
        return 2;
    } else if (strcmp(name, "ro.product.model") == 0) {
        strcpy(value, "PS Vita");
        return 7;
    } else if (strcmp(name, "ro.product.manufacturer") == 0) {
        strcpy(value, "Sony");
        return 4;
    }

    value[0] = '\0';
    return 0;
}

// Looper stubs
typedef struct ALooper ALooper;

ALooper *ALooper_forThread() {
    printf("Android: ALooper_forThread()\n");
    static int fake_looper = 0x44444444;
    return (ALooper *)&fake_looper;
}

ALooper *ALooper_prepare(int opts) {
    printf("Android: ALooper_prepare(%d)\n", opts);
    static int fake_looper = 0x55555555;
    return (ALooper *)&fake_looper;
}

int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    printf("Android: ALooper_pollOnce(%d)\n", timeoutMillis);
    return -1; // ALOOPER_POLL_TIMEOUT
}

// Additional common Android functions that games might use
void android_set_abort_message(const char *msg) {
    printf("Android: android_set_abort_message(%s)\n", msg);
}

// Initialize Android API bridge
void android_patch_init() {
    printf("Android: Android API bridge initialized (GTA SA Vita style)\n");
}
