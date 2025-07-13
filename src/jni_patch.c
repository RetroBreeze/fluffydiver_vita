#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vitasdk.h>

void *fake_env = NULL;
void *fake_context = NULL;

// Minimal JNI environment structure
static struct {
    void *functions[300];
} jni_env;

static void *jni_FindClass(void *env, const char *name) {
    printf("JNI: FindClass(%s)\n", name);
    return (void*)0x41414141;
}

static void *jni_GetMethodID(void *env, void *clazz, const char *name, const char *sig) {
    printf("JNI: GetMethodID(%s, %s)\n", name, sig);
    return (void*)0x42424242;
}

static void *jni_NewStringUTF(void *env, const char *str) {
    printf("JNI: NewStringUTF(%s)\n", str);
    return (void*)strdup(str);
}

void jni_init(void) {
    printf("Initializing JNI environment...\n");

    memset(&jni_env, 0, sizeof(jni_env));

    // Set up function pointers based on JNI spec
    jni_env.functions[6] = jni_FindClass;
    jni_env.functions[33] = jni_GetMethodID;
    jni_env.functions[167] = jni_NewStringUTF;

    fake_env = &jni_env;
    fake_context = (void*)0x12345678;

    printf("JNI environment initialized\n");
    printf("fake_env = %p, fake_context = %p\n", fake_env, fake_context);
}
