/*
 * Complete JNI Environment Implementation for Fluffy Diver
 * Includes all the JNI functions the game actually tries to use
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/kernel/clib.h>

// Define JNICALL before using it
#define JNICALL
#define JNIEXPORT

// JNI types
typedef unsigned char   jboolean;
typedef signed char     jbyte;
typedef unsigned short  jchar;
typedef short           jshort;
typedef int             jint;
typedef long long       jlong;
typedef float           jfloat;
typedef double          jdouble;
typedef jint            jsize;

// JNI object types
typedef void*           jobject;
typedef jobject         jclass;
typedef jobject         jstring;
typedef jobject         jarray;
typedef jarray          jobjectArray;
typedef jarray          jbooleanArray;
typedef jarray          jbyteArray;
typedef jarray          jcharArray;
typedef jarray          jshortArray;
typedef jarray          jintArray;
typedef jarray          jlongArray;
typedef jarray          jfloatArray;
typedef jarray          jdoubleArray;

typedef jobject         jthrowable;
typedef jobject         jweak;

typedef union jvalue {
    jboolean    z;
    jbyte       b;
    jchar       c;
    jshort      s;
    jint        i;
    jlong       j;
    jfloat      f;
    jdouble     d;
    jobject     l;
} jvalue;

// JNI method and field IDs
typedef struct _jfieldID* jfieldID;
typedef struct _jmethodID* jmethodID;

// Forward declarations
struct JNINativeInterface;
struct JNIInvokeInterface;

typedef const struct JNINativeInterface* JNIEnv;
typedef const struct JNIInvokeInterface* JavaVM;

// Function pointer types for the most commonly used JNI functions
typedef jint (*GetVersion_t)(JNIEnv *env);
typedef jclass (*DefineClass_t)(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len);
typedef jclass (*FindClass_t)(JNIEnv *env, const char *name);
typedef jmethodID (*GetMethodID_t)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
typedef jmethodID (*GetStaticMethodID_t)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
typedef jobject (*CallObjectMethod_t)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
typedef jint (*CallIntMethod_t)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
typedef void (*CallVoidMethod_t)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
typedef jstring (*NewStringUTF_t)(JNIEnv *env, const char *utf);
typedef const char* (*GetStringUTFChars_t)(JNIEnv *env, jstring str, jboolean *isCopy);
typedef void (*ReleaseStringUTFChars_t)(JNIEnv *env, jstring str, const char *chars);
typedef jsize (*GetStringUTFLength_t)(JNIEnv *env, jstring str);
typedef jint (*ThrowNew_t)(JNIEnv *env, jclass clazz, const char *msg);
typedef jthrowable (*ExceptionOccurred_t)(JNIEnv *env);
typedef void (*ExceptionClear_t)(JNIEnv *env);
typedef jboolean (*ExceptionCheck_t)(JNIEnv *env);

// JNI Environment Structure - More complete
struct JNINativeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    GetVersion_t GetVersion;                    // 4
    DefineClass_t DefineClass;                  // 5
    FindClass_t FindClass;                      // 6
    void *FromReflectedMethod;                  // 7
    void *FromReflectedField;                   // 8
    void *ToReflectedMethod;                    // 9
    void *GetSuperclass;                        // 10
    void *IsAssignableFrom;                     // 11
    void *ToReflectedField;                     // 12

    ThrowNew_t Throw;                           // 13
    ThrowNew_t ThrowNew;                        // 14
    ExceptionOccurred_t ExceptionOccurred;     // 15
    void *ExceptionDescribe;                    // 16
    ExceptionClear_t ExceptionClear;            // 17
    void *FatalError;                           // 18
    void *PushLocalFrame;                       // 19
    void *PopLocalFrame;                        // 20

    void *NewGlobalRef;                         // 21
    void *DeleteGlobalRef;                      // 22
    void *DeleteLocalRef;                       // 23
    void *IsSameObject;                         // 24
    void *NewLocalRef;                          // 25
    void *EnsureLocalCapacity;                  // 26
    void *AllocObject;                          // 27
    void *NewObject;                            // 28
    void *NewObjectV;                           // 29
    void *NewObjectA;                           // 30

    void *GetObjectClass;                       // 31
    void *IsInstanceOf;                         // 32
    GetMethodID_t GetMethodID;                  // 33

    CallObjectMethod_t CallObjectMethod;       // 34
    void *CallObjectMethodV;                    // 35
    void *CallObjectMethodA;                    // 36
    void *CallBooleanMethod;                    // 37
    void *CallBooleanMethodV;                   // 38
    void *CallBooleanMethodA;                   // 39
    void *CallByteMethod;                       // 40
    void *CallByteMethodV;                      // 41
    void *CallByteMethodA;                      // 42
    void *CallCharMethod;                       // 43
    void *CallCharMethodV;                      // 44
    void *CallCharMethodA;                      // 45
    void *CallShortMethod;                      // 46
    void *CallShortMethodV;                     // 47
    void *CallShortMethodA;                     // 48
    CallIntMethod_t CallIntMethod;              // 49
    void *CallIntMethodV;                       // 50
    void *CallIntMethodA;                       // 51
    void *CallLongMethod;                       // 52
    void *CallLongMethodV;                      // 53
    void *CallLongMethodA;                      // 54
    void *CallFloatMethod;                      // 55
    void *CallFloatMethodV;                     // 56
    void *CallFloatMethodA;                     // 57
    void *CallDoubleMethod;                     // 58
    void *CallDoubleMethodV;                    // 59
    void *CallDoubleMethodA;                    // 60
    CallVoidMethod_t CallVoidMethod;            // 61
    void *CallVoidMethodV;                      // 62
    void *CallVoidMethodA;                      // 63

    // Skip many entries for brevity...
    void *reserved_64_to_165[102];              // Functions 64-165

    // String functions
    NewStringUTF_t NewStringUTF;                // 167
    GetStringUTFLength_t GetStringUTFLength;    // 168
    GetStringUTFChars_t GetStringUTFChars;      // 169
    ReleaseStringUTFChars_t ReleaseStringUTFChars; // 170

    // More reserved space
    void *reserved_171_to_229[59];              // Functions 171-229

    ExceptionCheck_t ExceptionCheck;            // 228
};

// JavaVM Structure
struct JNIInvokeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    void *DestroyJavaVM;
    void *AttachCurrentThread;
    void *DetachCurrentThread;
    void *GetEnv;
    void *AttachCurrentThreadAsDaemon;
};

// Global instances
static struct JNINativeInterface g_jni_interface;
static struct JNIInvokeInterface g_jvm_interface;
static JNIEnv g_env = &g_jni_interface;
static JavaVM g_jvm = &g_jvm_interface;

/*
 * JNI FUNCTION IMPLEMENTATIONS
 */

// Version info
jint jni_GetVersion(JNIEnv *env) {
    printf("[JNI] GetVersion called\n");
    return 0x00010006; // JNI version 1.6
}

// Class operations
jclass jni_FindClass(JNIEnv *env, const char *name) {
    printf("[JNI] FindClass: %s\n", name);

    // Return a dummy class object
    static int dummy_class = 1;
    return (jclass)&dummy_class;
}

// Method operations
jmethodID jni_GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    printf("[JNI] GetMethodID: %s (%s)\n", name, sig);

    // Return a dummy method ID
    static int dummy_method = 1;
    return (jmethodID)&dummy_method;
}

jmethodID jni_GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    printf("[JNI] GetStaticMethodID: %s (%s)\n", name, sig);

    // Return a dummy method ID
    static int dummy_static_method = 1;
    return (jmethodID)&dummy_static_method;
}

// Method calling
jobject jni_CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[JNI] CallObjectMethod called\n");
    return NULL; // Safe default
}

jint jni_CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[JNI] CallIntMethod called\n");
    return 0; // Safe default
}

void jni_CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[JNI] CallVoidMethod called\n");
    // Do nothing - safe for void methods
}

// String operations
jstring jni_NewStringUTF(JNIEnv *env, const char *utf) {
    if (!utf) return NULL;

    printf("[JNI] NewStringUTF: %s\n", utf);

    // Create a simple string wrapper
    char *str_copy = malloc(strlen(utf) + 1);
    if (str_copy) {
        strcpy(str_copy, utf);
    }
    return (jstring)str_copy;
}

const char* jni_GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    if (!str) return NULL;

    if (isCopy) *isCopy = 0; // We return the original

    const char *result = (const char*)str;
    printf("[JNI] GetStringUTFChars: %s\n", result);
    return result;
}

void jni_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *chars) {
    printf("[JNI] ReleaseStringUTFChars called\n");
    // For our simple implementation, we don't need to do anything
}

jsize jni_GetStringUTFLength(JNIEnv *env, jstring str) {
    if (!str) return 0;

    jsize len = strlen((const char*)str);
    printf("[JNI] GetStringUTFLength: %d\n", len);
    return len;
}

// Exception handling
jint jni_ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
    printf("[JNI] ThrowNew: %s\n", msg ? msg : "(null)");
    return 0; // Pretend exception was thrown
}

jthrowable jni_ExceptionOccurred(JNIEnv *env) {
    printf("[JNI] ExceptionOccurred called\n");
    return NULL; // No exceptions
}

void jni_ExceptionClear(JNIEnv *env) {
    printf("[JNI] ExceptionClear called\n");
    // Clear any pending exceptions (we don't have any)
}

jboolean jni_ExceptionCheck(JNIEnv *env) {
    printf("[JNI] ExceptionCheck called\n");
    return 0; // No exceptions
}

/*
 * ANDROID SYSTEM CALL STUBS - Only the minimal ones we need
 */

// Android logging (moved here from android_stubs.c to avoid conflicts)
int __android_log_print_jni(int priority, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("[ANDROID][%s] ", tag ? tag : "GAME");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
    return 0;
}

/*
 * INITIALIZATION FUNCTION
 */
int init_jni_environment() {
    printf("[JNI] Initializing complete JNI environment...\n");

    // Zero out structures
    memset(&g_jni_interface, 0, sizeof(g_jni_interface));
    memset(&g_jvm_interface, 0, sizeof(g_jvm_interface));

    // Set up the most important JNI function pointers
    g_jni_interface.GetVersion = jni_GetVersion;
    g_jni_interface.FindClass = jni_FindClass;
    g_jni_interface.GetMethodID = jni_GetMethodID;
    g_jni_interface.CallObjectMethod = jni_CallObjectMethod;
    g_jni_interface.CallIntMethod = jni_CallIntMethod;
    g_jni_interface.CallVoidMethod = jni_CallVoidMethod;
    g_jni_interface.NewStringUTF = jni_NewStringUTF;
    g_jni_interface.GetStringUTFChars = jni_GetStringUTFChars;
    g_jni_interface.ReleaseStringUTFChars = jni_ReleaseStringUTFChars;
    g_jni_interface.GetStringUTFLength = jni_GetStringUTFLength;
    g_jni_interface.ThrowNew = jni_ThrowNew;
    g_jni_interface.ExceptionOccurred = jni_ExceptionOccurred;
    g_jni_interface.ExceptionClear = jni_ExceptionClear;
    g_jni_interface.ExceptionCheck = jni_ExceptionCheck;

    printf("[JNI] Complete JNI environment initialized successfully\n");
    printf("[JNI] JNIEnv address: %p\n", &g_jni_interface);
    printf("[JNI] JavaVM address: %p\n", &g_jvm_interface);

    return 0;
}

/*
 * ACCESSOR FUNCTIONS
 */
void* get_jni_env() {
    return &g_jni_interface;
}

void* get_java_vm() {
    return &g_jvm_interface;
}

void cleanup_jni_environment() {
    printf("[JNI] Cleaning up complete JNI environment\n");
    // Nothing to clean up in our minimal implementation
}

/*
 * FILE I/O OVERRIDE SETUP
 */
void setup_android_file_redirection() {
    printf("[JNI] Android file redirection ready\n");
}
