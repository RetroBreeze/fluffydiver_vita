#ifndef __JNI_PATCH_H__
#define __JNI_PATCH_H__

// JNI type definitions - EXACT from GTA SA Vita
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

void jni_init(void);

extern void *fake_env;
extern void *fake_context;

#endif
