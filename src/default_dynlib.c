#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>
#include <vitasdk.h>
#include <vitaGL.h>

#include "so_util.h"

// External debug function
extern void debugPrintf(const char *fmt, ...);

// Android API functions (from android_patch.c)
extern int __android_log_print(int prio, const char *tag, const char *fmt, ...);
extern int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args);
extern int __android_log_write(int prio, const char *tag, const char *text);
extern void *AAssetManager_open(void *mgr, const char *filename, int mode);
extern int AAsset_read(void *asset, void *buf, size_t count);
extern off_t AAsset_seek(void *asset, off_t offset, int whence);
extern off_t AAsset_getLength(void *asset);
extern void AAsset_close(void *asset);
extern int __system_property_get(const char *name, char *value);
extern void *ALooper_forThread(void);
extern void *ALooper_prepare(int opts);
extern int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData);
extern void android_set_abort_message(const char *msg);

// File I/O hooks
FILE *fopen_hook(const char *filename, const char *mode) {
    char path[512];

    // Redirect asset paths
    if (strstr(filename, "assets/") || strstr(filename, "/android_asset/")) {
        const char *p = strstr(filename, "assets/");
        if (!p) p = strstr(filename, "/android_asset/") + 14;
        else p += 7;

        snprintf(path, sizeof(path), "ux0:data/fluffydiver/%s", p);
    } else {
        snprintf(path, sizeof(path), "ux0:data/fluffydiver/%s", filename);
    }

    printf("fopen: %s -> %s\n", filename, path);
    return fopen(path, mode);
}

// Pthread stubs
int pthread_mutex_init_fake(void **mutex, const void *attr) {
    *mutex = calloc(1, sizeof(SceKernelLwMutexWork));
    return sceKernelCreateLwMutex(*mutex, "mutex", 0, 0, NULL);
}

int pthread_mutex_destroy_fake(void **mutex) {
    return sceKernelDeleteLwMutex(*mutex);
}

int pthread_mutex_lock_fake(void **mutex) {
    return sceKernelLockLwMutex(*mutex, 1, NULL);
}

int pthread_mutex_unlock_fake(void **mutex) {
    return sceKernelUnlockLwMutex(*mutex, 1);
}

// Import stub functions from so_util.c
extern int ret0();
extern int ret1();
extern int retminus1();
extern void *retNULL();

// JNI_OnLoad stub
int JNI_OnLoad(void *vm, void *reserved) {
    printf("JNI_OnLoad called\n");
    return 0x00010006; // JNI_VERSION_1_6
}

// Complete symbol table - GTA SA Vita style with Android API support
DynLibFunction default_dynlib[] = {
    // JNI
    {"JNI_OnLoad", (uintptr_t)&JNI_OnLoad},

    // Android logging
    {"__android_log_print", (uintptr_t)&__android_log_print},
    {"__android_log_vprint", (uintptr_t)&__android_log_vprint},
    {"__android_log_write", (uintptr_t)&__android_log_write},

    // Android Asset Manager
    {"AAssetManager_open", (uintptr_t)&AAssetManager_open},
    {"AAsset_read", (uintptr_t)&AAsset_read},
    {"AAsset_seek", (uintptr_t)&AAsset_seek},
    {"AAsset_getLength", (uintptr_t)&AAsset_getLength},
    {"AAsset_close", (uintptr_t)&AAsset_close},

    // Android System Properties
    {"__system_property_get", (uintptr_t)&__system_property_get},

    // Android Looper
    {"ALooper_forThread", (uintptr_t)&ALooper_forThread},
    {"ALooper_prepare", (uintptr_t)&ALooper_prepare},
    {"ALooper_pollOnce", (uintptr_t)&ALooper_pollOnce},

    // Android Misc
    {"android_set_abort_message", (uintptr_t)&android_set_abort_message},

    // File I/O
    {"fopen", (uintptr_t)&fopen_hook},
    {"fclose", (uintptr_t)&fclose},
    {"fread", (uintptr_t)&fread},
    {"fwrite", (uintptr_t)&fwrite},
    {"fseek", (uintptr_t)&fseek},
    {"ftell", (uintptr_t)&ftell},
    {"feof", (uintptr_t)&feof},
    {"fflush", (uintptr_t)&fflush},
    {"ferror", (uintptr_t)&ferror},
    {"clearerr", (uintptr_t)&clearerr},
    {"fgetc", (uintptr_t)&fgetc},
    {"fputc", (uintptr_t)&fputc},
    {"fgets", (uintptr_t)&fgets},
    {"fputs", (uintptr_t)&fputs},
    {"fprintf", (uintptr_t)&fprintf},
    {"fscanf", (uintptr_t)&fscanf},
    {"fileno", (uintptr_t)&fileno},
    {"rewind", (uintptr_t)&rewind},

    // Memory
    {"malloc", (uintptr_t)&malloc},
    {"free", (uintptr_t)&free},
    {"calloc", (uintptr_t)&calloc},
    {"realloc", (uintptr_t)&realloc},
    {"memcpy", (uintptr_t)&sceClibMemcpy},
    {"memmove", (uintptr_t)&sceClibMemmove},
    {"memset", (uintptr_t)&sceClibMemset},
    {"memcmp", (uintptr_t)&sceClibMemcmp},
    {"memchr", (uintptr_t)&memchr},

    // String functions
    {"strlen", (uintptr_t)&strlen},
    {"strcpy", (uintptr_t)&strcpy},
    {"strcat", (uintptr_t)&strcat},
    {"strcmp", (uintptr_t)&strcmp},
    {"strncmp", (uintptr_t)&strncmp},
    {"strncpy", (uintptr_t)&strncpy},
    {"strncat", (uintptr_t)&strncat},
    {"strstr", (uintptr_t)&strstr},
    {"strchr", (uintptr_t)&strchr},
    {"strrchr", (uintptr_t)&strrchr},
    {"strpbrk", (uintptr_t)&strpbrk},
    {"strspn", (uintptr_t)&strspn},
    {"strcspn", (uintptr_t)&strcspn},
    {"strtok", (uintptr_t)&strtok},
    {"strtok_r", (uintptr_t)&strtok_r},
    {"strdup", (uintptr_t)&strdup},
    {"strndup", (uintptr_t)&strndup},
    {"strcasecmp", (uintptr_t)&strcasecmp},
    {"strncasecmp", (uintptr_t)&strncasecmp},

    // String conversion
    {"atoi", (uintptr_t)&atoi},
    {"atol", (uintptr_t)&atol},
    {"atof", (uintptr_t)&atof},
    {"strtol", (uintptr_t)&strtol},
    {"strtoul", (uintptr_t)&strtoul},
    {"strtod", (uintptr_t)&strtod},
    {"strtof", (uintptr_t)&strtof},

    // Pthread
    {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init_fake},
    {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_fake},
    {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_fake},
    {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_fake},
    {"pthread_create", (uintptr_t)&ret0},
    {"pthread_join", (uintptr_t)&ret0},
    {"pthread_detach", (uintptr_t)&ret0},
    {"pthread_exit", (uintptr_t)&ret0},
    {"pthread_self", (uintptr_t)&ret0},
    {"pthread_equal", (uintptr_t)&ret1},
    {"pthread_key_create", (uintptr_t)&pthread_key_create},
    {"pthread_key_delete", (uintptr_t)&pthread_key_delete},
    {"pthread_getspecific", (uintptr_t)&pthread_getspecific},
    {"pthread_setspecific", (uintptr_t)&pthread_setspecific},
    {"pthread_cond_init", (uintptr_t)&pthread_cond_init},
    {"pthread_cond_destroy", (uintptr_t)&pthread_cond_destroy},
    {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait},
    {"pthread_cond_signal", (uintptr_t)&pthread_cond_signal},
    {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast},
    {"pthread_cond_timedwait", (uintptr_t)&pthread_cond_timedwait},
    {"pthread_attr_init", (uintptr_t)&pthread_attr_init},
    {"pthread_attr_destroy", (uintptr_t)&pthread_attr_destroy},
    {"pthread_attr_setdetachstate", (uintptr_t)&pthread_attr_setdetachstate},
    {"pthread_attr_setstacksize", (uintptr_t)&pthread_attr_setstacksize},

    // Math functions
    {"sin", (uintptr_t)&sin},
    {"cos", (uintptr_t)&cos},
    {"tan", (uintptr_t)&tan},
    {"asin", (uintptr_t)&asin},
    {"acos", (uintptr_t)&acos},
    {"atan", (uintptr_t)&atan},
    {"atan2", (uintptr_t)&atan2},
    {"sinh", (uintptr_t)&sinh},
    {"cosh", (uintptr_t)&cosh},
    {"tanh", (uintptr_t)&tanh},
    {"sinf", (uintptr_t)&sinf},
    {"cosf", (uintptr_t)&cosf},
    {"tanf", (uintptr_t)&tanf},
    {"asinf", (uintptr_t)&asinf},
    {"acosf", (uintptr_t)&acosf},
    {"atanf", (uintptr_t)&atanf},
    {"atan2f", (uintptr_t)&atan2f},
    {"sinhf", (uintptr_t)&sinhf},
    {"coshf", (uintptr_t)&coshf},
    {"tanhf", (uintptr_t)&tanhf},
    {"exp", (uintptr_t)&exp},
    {"log", (uintptr_t)&log},
    {"log10", (uintptr_t)&log10},
    {"pow", (uintptr_t)&pow},
    {"sqrt", (uintptr_t)&sqrt},
    {"expf", (uintptr_t)&expf},
    {"logf", (uintptr_t)&logf},
    {"log10f", (uintptr_t)&log10f},
    {"powf", (uintptr_t)&powf},
    {"sqrtf", (uintptr_t)&sqrtf},
    {"ceil", (uintptr_t)&ceil},
    {"floor", (uintptr_t)&floor},
    {"fabs", (uintptr_t)&fabs},
    {"fmod", (uintptr_t)&fmod},
    {"ceilf", (uintptr_t)&ceilf},
    {"floorf", (uintptr_t)&floorf},
    {"fabsf", (uintptr_t)&fabsf},
    {"fmodf", (uintptr_t)&fmodf},
    {"modf", (uintptr_t)&modf},
    {"modff", (uintptr_t)&modff},
    {"frexp", (uintptr_t)&frexp},
    {"frexpf", (uintptr_t)&frexpf},
    {"ldexp", (uintptr_t)&ldexp},
    {"ldexpf", (uintptr_t)&ldexpf},

    // Time functions
    {"time", (uintptr_t)&time},
    {"gettimeofday", (uintptr_t)&gettimeofday},
    {"localtime", (uintptr_t)&localtime},
    {"gmtime", (uintptr_t)&gmtime},
    {"mktime", (uintptr_t)&mktime},
    {"strftime", (uintptr_t)&strftime},
    {"clock", (uintptr_t)&clock},
    {"difftime", (uintptr_t)&difftime},

    // Process/System functions
    {"getenv", (uintptr_t)&getenv},
    {"setenv", (uintptr_t)&setenv},
    {"unsetenv", (uintptr_t)&unsetenv},
    {"system", (uintptr_t)&ret0},
    {"getpid", (uintptr_t)&ret0},
    {"getuid", (uintptr_t)&ret0},
    {"geteuid", (uintptr_t)&ret0},
    {"getgid", (uintptr_t)&ret0},
    {"getegid", (uintptr_t)&ret0},

    // Random functions
    {"srand", (uintptr_t)&srand},
    {"rand", (uintptr_t)&rand},
    {"drand48", (uintptr_t)&drand48},
    {"srand48", (uintptr_t)&srand48},

    // Program control
    {"exit", (uintptr_t)&exit},
    {"abort", (uintptr_t)&abort},
    {"atexit", (uintptr_t)&atexit},

    // Error handling
    {"strerror", (uintptr_t)&strerror},
    {"perror", (uintptr_t)&perror},
    {"__errno", (uintptr_t)&__errno},

    // Signal handling
    {"signal", (uintptr_t)&signal},
    {"raise", (uintptr_t)&raise},

    // I/O formatting
    {"printf", (uintptr_t)&printf},
    {"sprintf", (uintptr_t)&sprintf},
    {"snprintf", (uintptr_t)&snprintf},
    {"vprintf", (uintptr_t)&vprintf},
    {"vsprintf", (uintptr_t)&vsprintf},
    {"vsnprintf", (uintptr_t)&vsnprintf},
    {"scanf", (uintptr_t)&scanf},
    {"sscanf", (uintptr_t)&sscanf},
    {"puts", (uintptr_t)&puts},
    {"putchar", (uintptr_t)&putchar},
    {"getchar", (uintptr_t)&getchar},

    // C++ support
    {"__cxa_finalize", (uintptr_t)&ret0},
    {"__cxa_begin_cleanup", (uintptr_t)&ret0},
    {"__cxa_pure_virtual", (uintptr_t)&ret0},
    {"__cxa_type_match", (uintptr_t)&ret0},
    {"__cxa_allocate_exception", (uintptr_t)&malloc},
    {"__cxa_throw", (uintptr_t)&ret0},
    {"__cxa_guard_acquire", (uintptr_t)&ret1},
    {"__cxa_guard_release", (uintptr_t)&ret0},
    {"__cxa_atexit", (uintptr_t)&ret0},
    {"__aeabi_atexit", (uintptr_t)&ret0},

    // ARM EABI support
    {"__gnu_Unwind_Find_exidx", (uintptr_t)&ret0},
    {"__aeabi_unwind_cpp_pr0", (uintptr_t)&ret0},
    {"__aeabi_unwind_cpp_pr1", (uintptr_t)&ret0},
    {"__aeabi_unwind_cpp_pr2", (uintptr_t)&ret0},
    {"__aeabi_idiv", (uintptr_t)&ret0},
    {"__aeabi_uidiv", (uintptr_t)&ret0},
    {"__aeabi_idivmod", (uintptr_t)&ret0},
    {"__aeabi_uidivmod", (uintptr_t)&ret0},

    // Stack protection
    {"__stack_chk_fail", (uintptr_t)&ret0},
    {"__stack_chk_guard", (uintptr_t)&ret0},

    // Assert
    {"__assert", (uintptr_t)&ret0},
    {"__assert2", (uintptr_t)&ret0},

    // Locale (remove if not available on Vita)
    {"setlocale", (uintptr_t)&ret0},
    {"localeconv", (uintptr_t)&ret0},

    // Character classification (use stubs if not available)
    {"isalnum", (uintptr_t)&isalnum},
    {"isalpha", (uintptr_t)&isalpha},
    {"isdigit", (uintptr_t)&isdigit},
    {"islower", (uintptr_t)&islower},
    {"isupper", (uintptr_t)&isupper},
    {"isspace", (uintptr_t)&isspace},
    {"ispunct", (uintptr_t)&ispunct},
    {"isprint", (uintptr_t)&isprint},
    {"isgraph", (uintptr_t)&isgraph},
    {"iscntrl", (uintptr_t)&iscntrl},
    {"isxdigit", (uintptr_t)&isxdigit},
    {"tolower", (uintptr_t)&tolower},
    {"toupper", (uintptr_t)&toupper},

    // OpenGL ES 2.0 functions
    {"glActiveTexture", (uintptr_t)&glActiveTexture},
    {"glAttachShader", (uintptr_t)&glAttachShader},
    {"glBindAttribLocation", (uintptr_t)&glBindAttribLocation},
    {"glBindBuffer", (uintptr_t)&glBindBuffer},
    {"glBindFramebuffer", (uintptr_t)&glBindFramebuffer},
    {"glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer},
    {"glBindTexture", (uintptr_t)&glBindTexture},
    {"glBlendColor", (uintptr_t)&ret0}, // Not in VitaGL
    {"glBlendEquation", (uintptr_t)&glBlendEquation},
    {"glBlendEquationSeparate", (uintptr_t)&glBlendEquationSeparate},
    {"glBlendFunc", (uintptr_t)&glBlendFunc},
    {"glBlendFuncSeparate", (uintptr_t)&glBlendFuncSeparate},
    {"glBufferData", (uintptr_t)&glBufferData},
    {"glBufferSubData", (uintptr_t)&glBufferSubData},
    {"glCheckFramebufferStatus", (uintptr_t)&glCheckFramebufferStatus},
    {"glClear", (uintptr_t)&glClear},
    {"glClearColor", (uintptr_t)&glClearColor},
    {"glClearDepthf", (uintptr_t)&glClearDepthf},
    {"glClearStencil", (uintptr_t)&glClearStencil},
    {"glColorMask", (uintptr_t)&glColorMask},
    {"glCompileShader", (uintptr_t)&glCompileShader},
    {"glCompressedTexImage2D", (uintptr_t)&glCompressedTexImage2D},
    {"glCompressedTexSubImage2D", (uintptr_t)&ret0}, // Not in VitaGL
    {"glCopyTexImage2D", (uintptr_t)&glCopyTexImage2D},
    {"glCopyTexSubImage2D", (uintptr_t)&glCopyTexSubImage2D},
    {"glCreateProgram", (uintptr_t)&glCreateProgram},
    {"glCreateShader", (uintptr_t)&glCreateShader},
    {"glCullFace", (uintptr_t)&glCullFace},
    {"glDeleteBuffers", (uintptr_t)&glDeleteBuffers},
    {"glDeleteFramebuffers", (uintptr_t)&glDeleteFramebuffers},
    {"glDeleteProgram", (uintptr_t)&glDeleteProgram},
    {"glDeleteRenderbuffers", (uintptr_t)&glDeleteRenderbuffers},
    {"glDeleteShader", (uintptr_t)&glDeleteShader},
    {"glDeleteTextures", (uintptr_t)&glDeleteTextures},
    {"glDepthFunc", (uintptr_t)&glDepthFunc},
    {"glDepthMask", (uintptr_t)&glDepthMask},
    {"glDepthRangef", (uintptr_t)&glDepthRangef},
    {"glDetachShader", (uintptr_t)&ret0}, // Not in VitaGL
    {"glDisable", (uintptr_t)&glDisable},
    {"glDisableVertexAttribArray", (uintptr_t)&glDisableVertexAttribArray},
    {"glDrawArrays", (uintptr_t)&glDrawArrays},
    {"glDrawElements", (uintptr_t)&glDrawElements},
    {"glEnable", (uintptr_t)&glEnable},
    {"glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray},
    {"glFinish", (uintptr_t)&glFinish},
    {"glFlush", (uintptr_t)&glFlush},
    {"glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer},
    {"glFramebufferTexture2D", (uintptr_t)&glFramebufferTexture2D},
    {"glFrontFace", (uintptr_t)&glFrontFace},
    {"glGenBuffers", (uintptr_t)&glGenBuffers},
    {"glGenFramebuffers", (uintptr_t)&glGenFramebuffers},
    {"glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers},
    {"glGenTextures", (uintptr_t)&glGenTextures},
    {"glGenerateMipmap", (uintptr_t)&glGenerateMipmap},
    {"glGetActiveAttrib", (uintptr_t)&glGetActiveAttrib},
    {"glGetActiveUniform", (uintptr_t)&glGetActiveUniform},
    {"glGetAttachedShaders", (uintptr_t)&glGetAttachedShaders},
    {"glGetAttribLocation", (uintptr_t)&glGetAttribLocation},
    {"glGetBooleanv", (uintptr_t)&glGetBooleanv},
    {"glGetBufferParameteriv", (uintptr_t)&glGetBufferParameteriv},
    {"glGetError", (uintptr_t)&glGetError},
    {"glGetFloatv", (uintptr_t)&glGetFloatv},
    {"glGetFramebufferAttachmentParameteriv", (uintptr_t)&glGetFramebufferAttachmentParameteriv},
    {"glGetIntegerv", (uintptr_t)&glGetIntegerv},
    {"glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog},
    {"glGetProgramiv", (uintptr_t)&glGetProgramiv},
    {"glGetRenderbufferParameteriv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog},
    {"glGetShaderPrecisionFormat", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetShaderSource", (uintptr_t)&glGetShaderSource},
    {"glGetShaderiv", (uintptr_t)&glGetShaderiv},
    {"glGetString", (uintptr_t)&glGetString},
    {"glGetTexParameterfv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetTexParameteriv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetUniformLocation", (uintptr_t)&glGetUniformLocation},
    {"glGetUniformfv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetUniformiv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glGetVertexAttribPointerv", (uintptr_t)&glGetVertexAttribPointerv},
    {"glGetVertexAttribfv", (uintptr_t)&glGetVertexAttribfv},
    {"glGetVertexAttribiv", (uintptr_t)&glGetVertexAttribiv},
    {"glHint", (uintptr_t)&glHint},
    {"glIsBuffer", (uintptr_t)&ret0}, // Not in VitaGL
    {"glIsEnabled", (uintptr_t)&glIsEnabled},
    {"glIsFramebuffer", (uintptr_t)&glIsFramebuffer},
    {"glIsProgram", (uintptr_t)&glIsProgram},
    {"glIsRenderbuffer", (uintptr_t)&glIsRenderbuffer},
    {"glIsShader", (uintptr_t)&ret0}, // Not in VitaGL
    {"glIsTexture", (uintptr_t)&glIsTexture},
    {"glLineWidth", (uintptr_t)&glLineWidth},
    {"glLinkProgram", (uintptr_t)&glLinkProgram},
    {"glPixelStorei", (uintptr_t)&glPixelStorei},
    {"glPolygonOffset", (uintptr_t)&glPolygonOffset},
    {"glReadPixels", (uintptr_t)&glReadPixels},
    {"glReleaseShaderCompiler", (uintptr_t)&glReleaseShaderCompiler},
    {"glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage},
    {"glSampleCoverage", (uintptr_t)&ret0}, // Not in VitaGL
    {"glScissor", (uintptr_t)&glScissor},
    {"glShaderBinary", (uintptr_t)&glShaderBinary},
    {"glShaderSource", (uintptr_t)&glShaderSource},
    {"glStencilFunc", (uintptr_t)&glStencilFunc},
    {"glStencilFuncSeparate", (uintptr_t)&glStencilFuncSeparate},
    {"glStencilMask", (uintptr_t)&glStencilMask},
    {"glStencilMaskSeparate", (uintptr_t)&glStencilMaskSeparate},
    {"glStencilOp", (uintptr_t)&glStencilOp},
    {"glStencilOpSeparate", (uintptr_t)&glStencilOpSeparate},
    {"glTexImage2D", (uintptr_t)&glTexImage2D},
    {"glTexParameterf", (uintptr_t)&glTexParameterf},
    {"glTexParameterfv", (uintptr_t)&ret0}, // Not in VitaGL
    {"glTexParameteri", (uintptr_t)&glTexParameteri},
    {"glTexParameteriv", (uintptr_t)&glTexParameteriv},
    {"glTexSubImage2D", (uintptr_t)&glTexSubImage2D},
    {"glUniform1f", (uintptr_t)&glUniform1f},
    {"glUniform1fv", (uintptr_t)&glUniform1fv},
    {"glUniform1i", (uintptr_t)&glUniform1i},
    {"glUniform1iv", (uintptr_t)&glUniform1iv},
    {"glUniform2f", (uintptr_t)&glUniform2f},
    {"glUniform2fv", (uintptr_t)&glUniform2fv},
    {"glUniform2i", (uintptr_t)&glUniform2i},
    {"glUniform2iv", (uintptr_t)&glUniform2iv},
    {"glUniform3f", (uintptr_t)&glUniform3f},
    {"glUniform3fv", (uintptr_t)&glUniform3fv},
    {"glUniform3i", (uintptr_t)&glUniform3i},
    {"glUniform3iv", (uintptr_t)&glUniform3iv},
    {"glUniform4f", (uintptr_t)&glUniform4f},
    {"glUniform4fv", (uintptr_t)&glUniform4fv},
    {"glUniform4i", (uintptr_t)&glUniform4i},
    {"glUniform4iv", (uintptr_t)&glUniform4iv},
    {"glUniformMatrix2fv", (uintptr_t)&glUniformMatrix2fv},
    {"glUniformMatrix3fv", (uintptr_t)&glUniformMatrix3fv},
    {"glUniformMatrix4fv", (uintptr_t)&glUniformMatrix4fv},
    {"glUseProgram", (uintptr_t)&glUseProgram},
    {"glValidateProgram", (uintptr_t)&ret0}, // Not in VitaGL
    {"glVertexAttrib1f", (uintptr_t)&glVertexAttrib1f},
    {"glVertexAttrib1fv", (uintptr_t)&glVertexAttrib1fv},
    {"glVertexAttrib2f", (uintptr_t)&glVertexAttrib2f},
    {"glVertexAttrib2fv", (uintptr_t)&glVertexAttrib2fv},
    {"glVertexAttrib3f", (uintptr_t)&glVertexAttrib3f},
    {"glVertexAttrib3fv", (uintptr_t)&glVertexAttrib3fv},
    {"glVertexAttrib4f", (uintptr_t)&glVertexAttrib4f},
    {"glVertexAttrib4fv", (uintptr_t)&glVertexAttrib4fv},
    {"glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer},
    {"glViewport", (uintptr_t)&glViewport},

    // OpenGL ES 1.x functions (for compatibility)
    {"glAlphaFunc", (uintptr_t)&glAlphaFunc},
    {"glClientActiveTexture", (uintptr_t)&glClientActiveTexture},
    {"glColor4f", (uintptr_t)&glColor4f},
    {"glColorPointer", (uintptr_t)&glColorPointer},
    {"glDisableClientState", (uintptr_t)&glDisableClientState},
    {"glEnableClientState", (uintptr_t)&glEnableClientState},
    {"glLoadIdentity", (uintptr_t)&glLoadIdentity},
    {"glLoadMatrixf", (uintptr_t)&glLoadMatrixf},
    {"glMatrixMode", (uintptr_t)&glMatrixMode},
    {"glMultMatrixf", (uintptr_t)&glMultMatrixf},
    {"glNormalPointer", (uintptr_t)&glNormalPointer},
    {"glOrthof", (uintptr_t)&glOrthof},
    {"glPopMatrix", (uintptr_t)&glPopMatrix},
    {"glPushMatrix", (uintptr_t)&glPushMatrix},
    {"glRotatef", (uintptr_t)&glRotatef},
    {"glScalef", (uintptr_t)&glScalef},
    {"glTexCoordPointer", (uintptr_t)&glTexCoordPointer},
    {"glTexEnvi", (uintptr_t)&glTexEnvi},
    {"glTranslatef", (uintptr_t)&glTranslatef},
    {"glVertexPointer", (uintptr_t)&glVertexPointer},

    // Additional OpenGL functions that games might use
    {"glGetTexEnviv", (uintptr_t)&ret0},
    {"glGetTexEnvfv", (uintptr_t)&ret0},
    {"glTexEnvf", (uintptr_t)&glTexEnvf},
    {"glTexEnvfv", (uintptr_t)&ret0},
    {"glFrustumf", (uintptr_t)&glFrustumf},
    {"glClipPlanef", (uintptr_t)&ret0},
    {"glGetClipPlanef", (uintptr_t)&ret0},
    {"glLightf", (uintptr_t)&ret0},
    {"glLightfv", (uintptr_t)&ret0},
    {"glGetLightfv", (uintptr_t)&ret0},
    {"glMaterialf", (uintptr_t)&ret0},
    {"glMaterialfv", (uintptr_t)&ret0},
    {"glGetMaterialfv", (uintptr_t)&ret0},
    {"glNormal3f", (uintptr_t)&ret0},
    {"glPointSize", (uintptr_t)&glPointSize},
    {"glFogf", (uintptr_t)&ret0},
    {"glFogfv", (uintptr_t)&ret0},
    {"glMultiTexCoord2f", (uintptr_t)&ret0},
    {"glMultiTexCoord4f", (uintptr_t)&ret0},
    {"glSampleCoveragef", (uintptr_t)&ret0},
    {"glDrawTexfOES", (uintptr_t)&ret0},
    {"glDrawTexiOES", (uintptr_t)&ret0},
    {"glDrawTexsOES", (uintptr_t)&ret0},
    {"glDrawTexxOES", (uintptr_t)&ret0},
    {"glDrawTexfvOES", (uintptr_t)&ret0},
    {"glDrawTexivOES", (uintptr_t)&ret0},
    {"glDrawTexsvOES", (uintptr_t)&ret0},
    {"glDrawTexxvOES", (uintptr_t)&ret0},

    // EGL functions (some games use these)
    {"eglGetDisplay", (uintptr_t)&ret0},
    {"eglInitialize", (uintptr_t)&ret1},
    {"eglTerminate", (uintptr_t)&ret1},
    {"eglChooseConfig", (uintptr_t)&ret1},
    {"eglCreateWindowSurface", (uintptr_t)&ret0},
    {"eglCreateContext", (uintptr_t)&ret0},
    {"eglMakeCurrent", (uintptr_t)&ret1},
    {"eglSwapBuffers", (uintptr_t)&ret1},
    {"eglDestroySurface", (uintptr_t)&ret1},
    {"eglDestroyContext", (uintptr_t)&ret1},
    {"eglGetError", (uintptr_t)&ret0},
    {"eglQueryString", (uintptr_t)&retNULL},
    {"eglGetProcAddress", (uintptr_t)&retNULL},
};

size_t default_dynlib_size = sizeof(default_dynlib) / sizeof(DynLibFunction);

// Debug helper to print unresolved symbols
void print_unresolved_symbols() {
    debugPrintf("Total symbols in default_dynlib: %d\n", default_dynlib_size);
}
