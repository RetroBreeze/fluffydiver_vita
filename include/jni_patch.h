/*
 * jni_patch.h - JNI environment header for Fluffy Diver
 * Based on GTA SA Vita JNI implementation
 * Reference: https://github.com/TheOfficialFloW/gtasa_vita/blob/master/loader/jni_patch.h
 */

#ifndef __JNI_PATCH_H__
#define __JNI_PATCH_H__

// ===== JNI TYPE DEFINITIONS FIRST =====
// EXACT from GTA SA Vita - these MUST come before any function declarations

typedef void* jobject;
typedef jobject jclass;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jobjectArray;
typedef jarray jbooleanArray;
typedef jarray jbyteArray;
typedef jarray jcharArray;
typedef jarray jshortArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jarray jfloatArray;
typedef jarray jdoubleArray;
typedef jobject jthrowable;
typedef jobject jweak;

typedef union jvalue {
    unsigned char z;
    signed char b;
    unsigned short c;
    short s;
    int i;
    long long j;
    float f;
    double d;
    jobject l;
} jvalue;

typedef struct _jfieldID* jfieldID;
typedef struct _jmethodID* jmethodID;

typedef int jint;
typedef long long jlong;
typedef signed char jbyte;
typedef unsigned char jboolean;
typedef unsigned short jchar;
typedef short jshort;
typedef float jfloat;
typedef double jdouble;
typedef int jsize;

#define JNI_FALSE 0
#define JNI_TRUE 1

typedef struct JNINativeInterface* JNIEnv;
typedef struct JavaVM_ JavaVM;

// ===== JNI FUNCTION DECLARATIONS =====
// Now that types are defined, we can declare functions that use them

jstring jni_NewStringUTF(void *env, const char *str);
const char *jni_GetStringUTFChars(void *env, jstring string, jboolean *isCopy);
void jni_ReleaseStringUTFChars(void *env, jstring string, const char *utf);
jsize jni_GetStringUTFLength(void *env, jstring string);

jclass jni_FindClass(void *env, const char *name);
jmethodID jni_GetMethodID(void *env, jclass clazz, const char *name, const char *sig);
jmethodID jni_GetStaticMethodID(void *env, jclass clazz, const char *name, const char *sig);

jobject jni_NewObject(void *env, jclass clazz, jmethodID methodID, ...);
jclass jni_GetObjectClass(void *env, jobject obj);

jfieldID jni_GetFieldID(void *env, jclass clazz, const char *name, const char *sig);
jfieldID jni_GetStaticFieldID(void *env, jclass clazz, const char *name, const char *sig);

jobject jni_GetObjectField(void *env, jobject obj, jfieldID fieldID);
jint jni_GetIntField(void *env, jobject obj, jfieldID fieldID);
void jni_SetIntField(void *env, jobject obj, jfieldID fieldID, jint value);
void jni_SetObjectField(void *env, jobject obj, jfieldID fieldID, jobject value);

jobject jni_GetStaticObjectField(void *env, jclass clazz, jfieldID fieldID);
jint jni_GetStaticIntField(void *env, jclass clazz, jfieldID fieldID);
void jni_SetStaticIntField(void *env, jclass clazz, jfieldID fieldID, jint value);
void jni_SetStaticObjectField(void *env, jclass clazz, jfieldID fieldID, jobject value);

jobject jni_CallObjectMethod(void *env, jobject obj, jmethodID methodID, ...);
jint jni_CallIntMethod(void *env, jobject obj, jmethodID methodID, ...);
void jni_CallVoidMethod(void *env, jobject obj, jmethodID methodID, ...);

jobject jni_CallStaticObjectMethod(void *env, jclass clazz, jmethodID methodID, ...);
jint jni_CallStaticIntMethod(void *env, jclass clazz, jmethodID methodID, ...);
void jni_CallStaticVoidMethod(void *env, jclass clazz, jmethodID methodID, ...);

jsize jni_GetArrayLength(void *env, jarray array);
jobjectArray jni_NewObjectArray(void *env, jsize length, jclass elementClass, jobject initialElement);
jobject jni_GetObjectArrayElement(void *env, jobjectArray array, jsize index);
void jni_SetObjectArrayElement(void *env, jobjectArray array, jsize index, jobject value);

jintArray jni_NewIntArray(void *env, jsize length);
jint *jni_GetIntArrayElements(void *env, jintArray array, jboolean *isCopy);
void jni_ReleaseIntArrayElements(void *env, jintArray array, jint *elems, jint mode);

jfloatArray jni_NewFloatArray(void *env, jsize length);
jfloat *jni_GetFloatArrayElements(void *env, jfloatArray array, jboolean *isCopy);
void jni_ReleaseFloatArrayElements(void *env, jfloatArray array, jfloat *elems, jint mode);

jint jni_Throw(void *env, jthrowable obj);
jint jni_ThrowNew(void *env, jclass clazz, const char *message);
jthrowable jni_ExceptionOccurred(void *env);
void jni_ExceptionDescribe(void *env);
void jni_ExceptionClear(void *env);
jboolean jni_ExceptionCheck(void *env);

jobject jni_NewGlobalRef(void *env, jobject obj);
void jni_DeleteGlobalRef(void *env, jobject globalRef);
void jni_DeleteLocalRef(void *env, jobject localRef);
jweak jni_NewWeakGlobalRef(void *env, jobject obj);
void jni_DeleteWeakGlobalRef(void *env, jweak obj);

jint jni_MonitorEnter(void *env, jobject obj);
jint jni_MonitorExit(void *env, jobject obj);
jint jni_GetJavaVM(void *env, JavaVM **vm);
jint jni_GetVersion(void *env);

// ===== INITIALIZATION FUNCTION =====
void jni_init(void);

// ===== GLOBAL VARIABLES =====
extern void *fake_env;
extern void *fake_context;

#endif // __JNI_PATCH_H__
