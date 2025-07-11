/*
 * FIXED FalsoJNI - Exact Android JNI Interface Layout
 * This matches the real Android JNI structure exactly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/kernel/clib.h>

// JNI Core Types
typedef unsigned char   jboolean;
typedef signed char     jbyte;
typedef unsigned short  jchar;
typedef short           jshort;
typedef int             jint;
typedef long long       jlong;
typedef float           jfloat;
typedef double          jdouble;
typedef jint            jsize;

#define JNI_FALSE 0
#define JNI_TRUE 1

// JNI Object Types
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

// JNI Method and Field IDs
typedef struct _jfieldID* jfieldID;
typedef struct _jmethodID* jmethodID;

// JNI Value Union
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

// Forward Declarations
struct JNINativeInterface;
struct JNIInvokeInterface;

typedef const struct JNINativeInterface* JNIEnv;
typedef const struct JNIInvokeInterface* JavaVM;

// EXACT Android JNI Interface Structure (229 functions)
struct JNINativeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    // Version functions
    jint (*GetVersion)(JNIEnv *env);

    // Class operations
    jclass (*DefineClass)(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize len);
    jclass (*FindClass)(JNIEnv *env, const char *name);
    jmethodID (*FromReflectedMethod)(JNIEnv *env, jobject method);
    jfieldID (*FromReflectedField)(JNIEnv *env, jobject field);
    jobject (*ToReflectedMethod)(JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic);

    jclass (*GetSuperclass)(JNIEnv *env, jclass sub);
    jboolean (*IsAssignableFrom)(JNIEnv *env, jclass sub, jclass sup);
    jobject (*ToReflectedField)(JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic);

    // Exception operations
    jint (*Throw)(JNIEnv *env, jthrowable obj);
    jint (*ThrowNew)(JNIEnv *env, jclass clazz, const char *msg);
    jthrowable (*ExceptionOccurred)(JNIEnv *env);
    void (*ExceptionDescribe)(JNIEnv *env);
    void (*ExceptionClear)(JNIEnv *env);
    void (*FatalError)(JNIEnv *env, const char *msg);

    // Reference operations
    jint (*PushLocalFrame)(JNIEnv *env, jint capacity);
    jobject (*PopLocalFrame)(JNIEnv *env, jobject result);

    jobject (*NewGlobalRef)(JNIEnv *env, jobject lobj);
    void (*DeleteGlobalRef)(JNIEnv *env, jobject gref);
    void (*DeleteLocalRef)(JNIEnv *env, jobject obj);
    jboolean (*IsSameObject)(JNIEnv *env, jobject obj1, jobject obj2);
    jobject (*NewLocalRef)(JNIEnv *env, jobject ref);
    jint (*EnsureLocalCapacity)(JNIEnv *env, jint capacity);

    // Object operations
    jobject (*AllocObject)(JNIEnv *env, jclass clazz);
    jobject (*NewObject)(JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject (*NewObjectV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject (*NewObjectA)(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jclass (*GetObjectClass)(JNIEnv *env, jobject obj);
    jboolean (*IsInstanceOf)(JNIEnv *env, jobject obj, jclass clazz);

    // Method operations
    jmethodID (*GetMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);

    // Method calling - Object
    jobject (*CallObjectMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jobject (*CallObjectMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jobject (*CallObjectMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    // Method calling - Boolean
    jboolean (*CallBooleanMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jboolean (*CallBooleanMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jboolean (*CallBooleanMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    // Method calling - Byte
    jbyte (*CallByteMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jbyte (*CallByteMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jbyte (*CallByteMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Char
    jchar (*CallCharMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jchar (*CallCharMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jchar (*CallCharMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Short
    jshort (*CallShortMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jshort (*CallShortMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jshort (*CallShortMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Int
    jint (*CallIntMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jint (*CallIntMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jint (*CallIntMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Long
    jlong (*CallLongMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jlong (*CallLongMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jlong (*CallLongMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Float
    jfloat (*CallFloatMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jfloat (*CallFloatMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jfloat (*CallFloatMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Double
    jdouble (*CallDoubleMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jdouble (*CallDoubleMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jdouble (*CallDoubleMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    // Method calling - Void
    void (*CallVoidMethod)(JNIEnv *env, jobject obj, jmethodID methodID, ...);
    void (*CallVoidMethodV)(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    void (*CallVoidMethodA)(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    // Skip 64-166 for brevity - fill with NULLs
    void* padding[103];

    // String operations (positions 167-170)
    jstring (*NewStringUTF)(JNIEnv *env, const char *utf);
    jsize (*GetStringUTFLength)(JNIEnv *env, jstring str);
    const char* (*GetStringUTFChars)(JNIEnv *env, jstring str, jboolean *isCopy);
    void (*ReleaseStringUTFChars)(JNIEnv *env, jstring str, const char* chars);

    // More padding to position 228
    void* more_padding[57];

    // Exception check (position 228)
    jboolean (*ExceptionCheck)(JNIEnv *env);
};

// JavaVM Interface
struct JNIInvokeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    jint (*DestroyJavaVM)(JavaVM *vm);
    jint (*AttachCurrentThread)(JavaVM *vm, void **penv, void *args);
    jint (*DetachCurrentThread)(JavaVM *vm);
    jint (*GetEnv)(JavaVM *vm, void **penv, jint version);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args);
};

// Global instances
static struct JNINativeInterface fixed_jni_interface;
static struct JNIInvokeInterface fixed_vm_interface;

/*
 * SAFE FUNCTION IMPLEMENTATIONS
 */

static jint safe_GetVersion(JNIEnv *env) {
    printf("[FIXED-JNI] GetVersion called\n");
    return 0x00010006; // JNI version 1.6
}

static jclass safe_FindClass(JNIEnv *env, const char *name) {
    printf("[FIXED-JNI] FindClass: %s\n", name);
    return (jclass)0x12345678; // Dummy class
}

static jmethodID safe_GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    printf("[FIXED-JNI] GetMethodID: %s (%s)\n", name, sig);
    return (jmethodID)0x87654321; // Dummy method ID
}

static jobject safe_CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FIXED-JNI] CallObjectMethod called\n");
    return NULL;
}

static jint safe_CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FIXED-JNI] CallIntMethod called\n");
    return 0;
}

static void safe_CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FIXED-JNI] CallVoidMethod called\n");
}

static jstring safe_NewStringUTF(JNIEnv *env, const char *utf) {
    printf("[FIXED-JNI] NewStringUTF: %s\n", utf ? utf : "(null)");
    return (jstring)0x11111111; // Dummy string
}

static jsize safe_GetStringUTFLength(JNIEnv *env, jstring str) {
    printf("[FIXED-JNI] GetStringUTFLength called\n");
    return 0;
}

static const char* safe_GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    printf("[FIXED-JNI] GetStringUTFChars called\n");
    if (isCopy) *isCopy = JNI_FALSE;
    return "dummy_string";
}

static void safe_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char* chars) {
    printf("[FIXED-JNI] ReleaseStringUTFChars called\n");
}

static jint safe_ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
    printf("[FIXED-JNI] ThrowNew: %s\n", msg ? msg : "(null)");
    return 0;
}

static jthrowable safe_ExceptionOccurred(JNIEnv *env) {
    printf("[FIXED-JNI] ExceptionOccurred called\n");
    return NULL;
}

static void safe_ExceptionClear(JNIEnv *env) {
    printf("[FIXED-JNI] ExceptionClear called\n");
}

static jboolean safe_ExceptionCheck(JNIEnv *env) {
    printf("[FIXED-JNI] ExceptionCheck called\n");
    return JNI_FALSE;
}

/*
 * FIXED FALSOJNI INITIALIZATION
 */
int fixed_falsojni_init() {
    printf("[FIXED-JNI] === INITIALIZING FIXED FALSOJNI ===\n");

    // Clear structures
    memset(&fixed_jni_interface, 0, sizeof(fixed_jni_interface));
    memset(&fixed_vm_interface, 0, sizeof(fixed_vm_interface));

    // Set up function pointers in EXACT positions
    fixed_jni_interface.GetVersion = safe_GetVersion;
    fixed_jni_interface.FindClass = safe_FindClass;
    fixed_jni_interface.GetMethodID = safe_GetMethodID;
    fixed_jni_interface.CallObjectMethod = safe_CallObjectMethod;
    fixed_jni_interface.CallIntMethod = safe_CallIntMethod;
    fixed_jni_interface.CallVoidMethod = safe_CallVoidMethod;
    fixed_jni_interface.NewStringUTF = safe_NewStringUTF;
    fixed_jni_interface.GetStringUTFLength = safe_GetStringUTFLength;
    fixed_jni_interface.GetStringUTFChars = safe_GetStringUTFChars;
    fixed_jni_interface.ReleaseStringUTFChars = safe_ReleaseStringUTFChars;
    fixed_jni_interface.ThrowNew = safe_ThrowNew;
    fixed_jni_interface.ExceptionOccurred = safe_ExceptionOccurred;
    fixed_jni_interface.ExceptionClear = safe_ExceptionClear;
    fixed_jni_interface.ExceptionCheck = safe_ExceptionCheck;

    printf("[FIXED-JNI] Fixed FalsoJNI initialized successfully\n");
    printf("[FIXED-JNI] JNI Interface size: %zu bytes\n", sizeof(fixed_jni_interface));
    printf("[FIXED-JNI] JNIEnv address: %p\n", &fixed_jni_interface);
    printf("[FIXED-JNI] JavaVM address: %p\n", &fixed_vm_interface);

    return 0;
}

void fixed_falsojni_cleanup() {
    printf("[FIXED-JNI] Fixed FalsoJNI cleanup\n");
}

JNIEnv* fixed_falsojni_get_env() {
    return &fixed_jni_interface;
}

JavaVM* fixed_falsojni_get_vm() {
    return &fixed_vm_interface;
}

/*
 * COMPATIBILITY WRAPPERS
 */
int setup_enhanced_jni_environment() {
    return fixed_falsojni_init();
}

void cleanup_enhanced_jni_environment() {
    fixed_falsojni_cleanup();
}

void* get_jni_env() {
    return fixed_falsojni_get_env();
}

void* get_java_vm() {
    return fixed_falsojni_get_vm();
}

// Keep the old functions for backwards compatibility
JNIEnv* falsojni_get_env() {
    return fixed_falsojni_get_env();
}

JavaVM* falsojni_get_vm() {
    return fixed_falsojni_get_vm();
}
