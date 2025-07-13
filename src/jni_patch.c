/*
 * jni_patch.c - Based on GTA SA Vita by TheOfficialFloW
 * Simplified JNI environment implementation
 */

#include <psp2/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Import stub functions from so_util.c
extern int ret0();
extern int ret1();
extern int retminus1();
extern void *retNULL();

// Debug stub for unimplemented functions
static void *jni_debug_stub(void *env, ...) {
    printf("JNI: DEBUG - Unimplemented function called\n");
    return NULL;
}

// JNI method implementations with explicit debug output
static void *jni_FindClass(void *env, const char *name) {
    printf("JNI: FindClass(%s)\n", name);
    return (void *)0x41414141;
}

static void *jni_GetMethodID(void *env, void *clazz, const char *name, const char *sig) {
    printf("JNI: GetMethodID(clazz=%p, name=%s, sig=%s)\n", clazz, name, sig);
    return (void *)0x42424242;
}

static void *jni_GetStaticMethodID(void *env, void *clazz, const char *name, const char *sig) {
    printf("JNI: GetStaticMethodID(clazz=%p, name=%s, sig=%s)\n", clazz, name, sig);
    return (void *)0x43434343;
}

static void *jni_NewStringUTF(void *env, const char *str) {
    printf("JNI: NewStringUTF(%s)\n", str);
    return (void *)strdup(str);
}

static const char *jni_GetStringUTFChars(void *env, void *string, void *isCopy) {
    printf("JNI: GetStringUTFChars(string=%p, isCopy=%p)\n", string, isCopy);
    return "dummy_string";
}

static void jni_ReleaseStringUTFChars(void *env, void *string, const char *utf) {
    printf("JNI: ReleaseStringUTFChars(string=%p, utf=%s)\n", string, utf);
}

static void *jni_CallStaticObjectMethod(void *env, void *clazz, void *methodID, ...) {
    printf("JNI: CallStaticObjectMethod(clazz=%p, methodID=%p)\n", clazz, methodID);
    return (void *)0x44444444;
}

static int jni_CallStaticIntMethod(void *env, void *clazz, void *methodID, ...) {
    printf("JNI: CallStaticIntMethod(clazz=%p, methodID=%p)\n", clazz, methodID);
    return 0;
}

static void jni_CallStaticVoidMethod(void *env, void *clazz, void *methodID, ...) {
    printf("JNI: CallStaticVoidMethod(clazz=%p, methodID=%p)\n", clazz, methodID);
}

static void *jni_CallObjectMethod(void *env, void *obj, void *methodID, ...) {
    printf("JNI: CallObjectMethod(obj=%p, methodID=%p)\n", obj, methodID);
    return (void *)0x45454545;
}

static int jni_CallIntMethod(void *env, void *obj, void *methodID, ...) {
    printf("JNI: CallIntMethod(obj=%p, methodID=%p)\n", obj, methodID);
    return 0;
}

static void jni_CallVoidMethod(void *env, void *obj, void *methodID, ...) {
    printf("JNI: CallVoidMethod(obj=%p, methodID=%p)\n", obj, methodID);
}

static void *jni_GetObjectClass(void *env, void *obj) {
    printf("JNI: GetObjectClass(obj=%p)\n", obj);
    return (void *)0x46464646;
}

static void *jni_GetFieldID(void *env, void *clazz, const char *name, const char *sig) {
    printf("JNI: GetFieldID(clazz=%p, name=%s, sig=%s)\n", clazz, name, sig);
    return (void *)0x47474747;
}

static void *jni_GetStaticFieldID(void *env, void *clazz, const char *name, const char *sig) {
    printf("JNI: GetStaticFieldID(clazz=%p, name=%s, sig=%s)\n", clazz, name, sig);
    return (void *)0x48484848;
}

static void *jni_GetObjectField(void *env, void *obj, void *fieldID) {
    printf("JNI: GetObjectField(obj=%p, fieldID=%p)\n", obj, fieldID);
    return (void *)0x49494949;
}

static int jni_GetIntField(void *env, void *obj, void *fieldID) {
    printf("JNI: GetIntField(obj=%p, fieldID=%p)\n", obj, fieldID);
    return 0;
}

static void jni_SetIntField(void *env, void *obj, void *fieldID, int value) {
    printf("JNI: SetIntField(obj=%p, fieldID=%p, value=%d)\n", obj, fieldID, value);
}

static void jni_SetObjectField(void *env, void *obj, void *fieldID, void *value) {
    printf("JNI: SetObjectField(obj=%p, fieldID=%p, value=%p)\n", obj, fieldID, value);
}

static void *jni_NewObject(void *env, void *clazz, void *methodID, ...) {
    printf("JNI: NewObject(clazz=%p, methodID=%p)\n", clazz, methodID);
    return (void *)0x50505050;
}

static void *jni_GetStaticObjectField(void *env, void *clazz, void *fieldID) {
    printf("JNI: GetStaticObjectField(clazz=%p, fieldID=%p)\n", clazz, fieldID);
    return (void *)0x51515151;
}

static int jni_GetStaticIntField(void *env, void *clazz, void *fieldID) {
    printf("JNI: GetStaticIntField(clazz=%p, fieldID=%p)\n", clazz, fieldID);
    return 0;
}

static void jni_SetStaticIntField(void *env, void *clazz, void *fieldID, int value) {
    printf("JNI: SetStaticIntField(clazz=%p, fieldID=%p, value=%d)\n", clazz, fieldID, value);
}

static void jni_SetStaticObjectField(void *env, void *clazz, void *fieldID, void *value) {
    printf("JNI: SetStaticObjectField(clazz=%p, fieldID=%p, value=%p)\n", clazz, fieldID, value);
}

// Simplified JNI function table
static void *jni_functions[232]; // Standard JNI has 232 functions

// Initialize the JNI function table
static void init_jni_functions() {
    printf("JNI: Initializing function table with 232 functions\n");

    // Initialize all functions to debug stub
    for (int i = 0; i < 232; i++) {
        jni_functions[i] = jni_debug_stub;
    }

    // Set up the important functions we actually implement
    jni_functions[6] = jni_FindClass;           // FindClass
    jni_functions[28] = jni_NewObject;          // NewObject
    jni_functions[31] = jni_GetObjectClass;     // GetObjectClass
    jni_functions[33] = jni_GetMethodID;        // GetMethodID
    jni_functions[34] = jni_CallObjectMethod;   // CallObjectMethod
    jni_functions[49] = jni_CallIntMethod;      // CallIntMethod
    jni_functions[61] = jni_CallVoidMethod;     // CallVoidMethod
    jni_functions[94] = jni_GetFieldID;         // GetFieldID
    jni_functions[95] = jni_GetObjectField;     // GetObjectField
    jni_functions[100] = jni_GetIntField;       // GetIntField
    jni_functions[104] = jni_SetObjectField;    // SetObjectField
    jni_functions[109] = jni_SetIntField;       // SetIntField
    jni_functions[112] = jni_GetStaticMethodID; // GetStaticMethodID
    jni_functions[113] = jni_CallStaticObjectMethod; // CallStaticObjectMethod
    jni_functions[128] = jni_CallStaticIntMethod;     // CallStaticIntMethod
    jni_functions[141] = jni_CallStaticVoidMethod;    // CallStaticVoidMethod
    jni_functions[144] = jni_GetStaticFieldID;        // GetStaticFieldID
    jni_functions[145] = jni_GetStaticObjectField;    // GetStaticObjectField
    jni_functions[150] = jni_GetStaticIntField;       // GetStaticIntField
    jni_functions[154] = jni_SetStaticObjectField;    // SetStaticObjectField
    jni_functions[159] = jni_SetStaticIntField;       // SetStaticIntField
    jni_functions[169] = jni_NewStringUTF;            // NewStringUTF
    jni_functions[171] = jni_GetStringUTFChars;       // GetStringUTFChars
    jni_functions[172] = jni_ReleaseStringUTFChars;   // ReleaseStringUTFChars

    printf("JNI: Function table initialized with proper function pointers\n");
}

static struct {
    void *functions;
} jni_env;

void *fake_env = &jni_env;

// Create fake context object
static int fake_java_object = 0x50505050;
void *fake_context = &fake_java_object;

void jni_init() {
    printf("JNI: Starting JNI initialization...\n");

    printf("JNI: About to initialize function table...\n");
    // Initialize the function table
    init_jni_functions();
    printf("JNI: Function table initialization completed\n");

    printf("JNI: About to set up JNI environment...\n");
    // Set up the JNI environment
    jni_env.functions = jni_functions;
    printf("JNI: JNI environment setup completed\n");

    printf("JNI: Environment initialized with comprehensive logging\n");
    printf("JNI: fake_env = %p, fake_context = %p\n", fake_env, fake_context);
    printf("JNI: All function calls will be logged for debugging\n");
    printf("JNI: Initialization complete!\n");
}
