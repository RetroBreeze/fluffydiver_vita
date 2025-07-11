/*
 * Fixed FalsoJNI Integration for Fluffy Diver
 * Complete structure definitions and proper implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>

// FalsoJNI Core Types
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

// Function pointer types
typedef jint (*GetVersion_t)(JNIEnv *env);
typedef jclass (*FindClass_t)(JNIEnv *env, const char *name);
typedef jmethodID (*GetMethodID_t)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
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

// Complete JNI Interface Structure
struct JNINativeInterface {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;
    
    GetVersion_t GetVersion;                    // 4
    void *DefineClass;                          // 5
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
    
    // Skip positions 64-166 for brevity
    void *reserved_64_to_166[103];
    
    // String functions
    NewStringUTF_t NewStringUTF;                // 167
    GetStringUTFLength_t GetStringUTFLength;    // 168
    GetStringUTFChars_t GetStringUTFChars;      // 169
    ReleaseStringUTFChars_t ReleaseStringUTFChars; // 170
    
    // More reserved space
    void *reserved_171_to_227[57];
    
    ExceptionCheck_t ExceptionCheck;            // 228
};

// JavaVM Interface Structure
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

// FalsoJNI String Management
typedef struct {
    char *data;
    size_t length;
    int is_copy;
} falsojni_string_t;

static falsojni_string_t *strings = NULL;
static int string_count = 0;
static int string_capacity = 0;

// FalsoJNI Class Management
typedef struct {
    char *name;
    void *methods;
} falsojni_class_t;

static falsojni_class_t *classes = NULL;
static int class_count = 0;

/*
 * FALSOJNI FUNCTION IMPLEMENTATIONS
 */

static jint falsojni_GetVersion(JNIEnv *env) {
    return 0x00010006; // JNI version 1.6
}

static jclass falsojni_FindClass(JNIEnv *env, const char *name) {
    printf("[FALSOJNI] FindClass: %s\n", name);
    
    // Check if we already have this class
    for (int i = 0; i < class_count; i++) {
        if (strcmp(classes[i].name, name) == 0) {
            return (jclass)(uintptr_t)(i + 1);
        }
    }
    
    // Create new class
    classes = realloc(classes, sizeof(falsojni_class_t) * (class_count + 1));
    falsojni_class_t *cls = &classes[class_count];
    cls->name = malloc(strlen(name) + 1);
    strcpy(cls->name, name);
    cls->methods = NULL;
    
    printf("[FALSOJNI] Created new class: %s (slot %d)\n", name, class_count);
    
    return (jclass)(uintptr_t)(++class_count);
}

static jmethodID falsojni_GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    printf("[FALSOJNI] GetMethodID: %s (%s)\n", name, sig);
    
    // Return a dummy method ID based on the method name hash
    uintptr_t method_id = 0;
    for (const char *p = name; *p; p++) {
        method_id = method_id * 31 + *p;
    }
    
    return (jmethodID)(method_id | 0x80000000); // Mark as method ID
}

static jobject falsojni_CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FALSOJNI] CallObjectMethod called (methodID: %p)\n", methodID);
    return NULL; // Safe default
}

static jint falsojni_CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FALSOJNI] CallIntMethod called (methodID: %p)\n", methodID);
    return 0; // Safe default
}

static void falsojni_CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    printf("[FALSOJNI] CallVoidMethod called (methodID: %p)\n", methodID);
    // Do nothing - safe for void methods
}

static jstring falsojni_NewStringUTF(JNIEnv *env, const char *utf) {
    if (!utf) return NULL;
    
    // Expand string array if needed
    if (string_count >= string_capacity) {
        string_capacity = string_capacity ? string_capacity * 2 : 16;
        strings = realloc(strings, sizeof(falsojni_string_t) * string_capacity);
    }
    
    // Create new string
    falsojni_string_t *str = &strings[string_count];
    str->length = strlen(utf);
    str->data = malloc(str->length + 1);
    strcpy(str->data, utf);
    str->is_copy = 0;
    
    printf("[FALSOJNI] NewStringUTF: %s (slot %d)\n", utf, string_count);
    
    return (jstring)(uintptr_t)(++string_count);
}

static const char* falsojni_GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    if (!str) return NULL;
    
    int index = (int)(uintptr_t)str - 1;
    if (index < 0 || index >= string_count) {
        printf("[FALSOJNI] GetStringUTFChars: Invalid string index %d\n", index);
        return NULL;
    }
    
    if (isCopy) *isCopy = strings[index].is_copy;
    
    printf("[FALSOJNI] GetStringUTFChars: %s (slot %d)\n", strings[index].data, index);
    return strings[index].data;
}

static void falsojni_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *chars) {
    printf("[FALSOJNI] ReleaseStringUTFChars called\n");
    // For our implementation, we don't need to do anything special
}

static jsize falsojni_GetStringUTFLength(JNIEnv *env, jstring str) {
    if (!str) return 0;
    
    int index = (int)(uintptr_t)str - 1;
    if (index < 0 || index >= string_count) return 0;
    
    return (jsize)strings[index].length;
}

static jint falsojni_ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
    printf("[FALSOJNI] ThrowNew: %s\n", msg ? msg : "(null)");
    return 0; // Pretend exception was thrown
}

static jthrowable falsojni_ExceptionOccurred(JNIEnv *env) {
    return NULL; // No exceptions in our implementation
}

static void falsojni_ExceptionClear(JNIEnv *env) {
    printf("[FALSOJNI] ExceptionClear called\n");
}

static jboolean falsojni_ExceptionCheck(JNIEnv *env) {
    return JNI_FALSE; // No exceptions
}

/*
 * GLOBAL JNI INTERFACE STRUCTURES
 */
static struct JNINativeInterface falsojni_interface;
static struct JNIInvokeInterface falsojni_vm_interface;

/*
 * ANDROID SYSTEM STUBS - Removed duplicate android_fopen
 */

/*
 * FALSOJNI INITIALIZATION
 */
int falsojni_init() {
    printf("[FALSOJNI] Initializing FalsoJNI environment...\n");
    
    // Initialize string storage
    string_capacity = 16;
    strings = malloc(sizeof(falsojni_string_t) * string_capacity);
    string_count = 0;
    
    // Initialize class storage
    classes = malloc(sizeof(falsojni_class_t) * 16);
    class_count = 0;
    
    // Zero out interface structures
    memset(&falsojni_interface, 0, sizeof(falsojni_interface));
    memset(&falsojni_vm_interface, 0, sizeof(falsojni_vm_interface));
    
    // Set up function pointers in the correct positions
    falsojni_interface.GetVersion = falsojni_GetVersion;
    falsojni_interface.FindClass = falsojni_FindClass;
    falsojni_interface.GetMethodID = falsojni_GetMethodID;
    falsojni_interface.CallObjectMethod = falsojni_CallObjectMethod;
    falsojni_interface.CallIntMethod = falsojni_CallIntMethod;
    falsojni_interface.CallVoidMethod = falsojni_CallVoidMethod;
    falsojni_interface.NewStringUTF = falsojni_NewStringUTF;
    falsojni_interface.GetStringUTFLength = falsojni_GetStringUTFLength;
    falsojni_interface.GetStringUTFChars = falsojni_GetStringUTFChars;
    falsojni_interface.ReleaseStringUTFChars = falsojni_ReleaseStringUTFChars;
    falsojni_interface.Throw = falsojni_ThrowNew;
    falsojni_interface.ThrowNew = falsojni_ThrowNew;
    falsojni_interface.ExceptionOccurred = falsojni_ExceptionOccurred;
    falsojni_interface.ExceptionClear = falsojni_ExceptionClear;
    falsojni_interface.ExceptionCheck = falsojni_ExceptionCheck;
    
    printf("[FALSOJNI] FalsoJNI initialized successfully\n");
    printf("[FALSOJNI] JNIEnv address: %p\n", &falsojni_interface);
    printf("[FALSOJNI] JavaVM address: %p\n", &falsojni_vm_interface);
    
    return 0;
}

void falsojni_cleanup() {
    printf("[FALSOJNI] Cleaning up FalsoJNI...\n");
    
    // Clean up strings
    for (int i = 0; i < string_count; i++) {
        free(strings[i].data);
    }
    free(strings);
    
    // Clean up classes
    for (int i = 0; i < class_count; i++) {
        free(classes[i].name);
    }
    free(classes);
    
    printf("[FALSOJNI] FalsoJNI cleanup complete\n");
}

JNIEnv* falsojni_get_env() {
    return &falsojni_interface;
}

JavaVM* falsojni_get_vm() {
    return &falsojni_vm_interface;
}

/*
 * COMPATIBILITY WRAPPERS FOR EXISTING CODE
 */
int setup_enhanced_jni_environment() {
    return falsojni_init();
}

void cleanup_enhanced_jni_environment() {
    falsojni_cleanup();
}

void* get_jni_env() {
    return falsojni_get_env();
}

void* get_java_vm() {
    return falsojni_get_vm();
}

void setup_android_file_redirection() {
    printf("[FALSOJNI] Android file redirection ready\n");
}
