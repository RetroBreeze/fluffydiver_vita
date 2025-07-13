#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <vitasdk.h>
#include <vitaGL.h>

#include "so_util.h"

// External debug function
extern void debugPrintf(const char *fmt, ...);

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

// Android log stub
int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("[%s] ", tag);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    return 0;
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

// Symbol table like GTA SA Vita
DynLibFunction default_dynlib[] = {
    // JNI
    {"JNI_OnLoad", (uintptr_t)&ret0},

    // Android logging
    {"__android_log_print", (uintptr_t)&__android_log_print},
    {"__android_log_write", (uintptr_t)&ret0},
    {"__android_log_vprint", (uintptr_t)&ret0},

    // File I/O
    {"fopen", (uintptr_t)&fopen_hook},
    {"fclose", (uintptr_t)&fclose},
    {"fread", (uintptr_t)&fread},
    {"fwrite", (uintptr_t)&fwrite},
    {"fseek", (uintptr_t)&fseek},
    {"ftell", (uintptr_t)&ftell},
    {"feof", (uintptr_t)&feof},
    {"fflush", (uintptr_t)&fflush},

    // Memory
    {"malloc", (uintptr_t)&malloc},
    {"free", (uintptr_t)&free},
    {"calloc", (uintptr_t)&calloc},
    {"realloc", (uintptr_t)&realloc},
    {"memcpy", (uintptr_t)&sceClibMemcpy},
    {"memmove", (uintptr_t)&sceClibMemmove},
    {"memset", (uintptr_t)&sceClibMemset},
    {"memcmp", (uintptr_t)&sceClibMemcmp},

    // String
    {"strlen", (uintptr_t)&strlen},
    {"strcpy", (uintptr_t)&strcpy},
    {"strcat", (uintptr_t)&strcat},
    {"strcmp", (uintptr_t)&strcmp},
    {"strncmp", (uintptr_t)&strncmp},
    {"strstr", (uintptr_t)&strstr},
    {"strchr", (uintptr_t)&strchr},
    {"strrchr", (uintptr_t)&strrchr},

    // Pthread
    {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init_fake},
    {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_fake},
    {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_fake},
    {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_fake},
    {"pthread_create", (uintptr_t)&ret0},
    {"pthread_join", (uintptr_t)&ret0},

    // Math
    {"sinf", (uintptr_t)&sinf},
    {"cosf", (uintptr_t)&cosf},
    {"tanf", (uintptr_t)&tanf},
    {"atan2f", (uintptr_t)&atan2f},
    {"sqrtf", (uintptr_t)&sqrtf},
    {"powf", (uintptr_t)&powf},
    {"floorf", (uintptr_t)&floorf},
    {"ceilf", (uintptr_t)&ceilf},
    {"sqrt", (uintptr_t)&sqrt},
    {"sin", (uintptr_t)&sin},
    {"cos", (uintptr_t)&cos},
    {"pow", (uintptr_t)&pow},
    {"asin", (uintptr_t)&asin},
    {"acosf", (uintptr_t)&acosf},

    // Time
    {"time", (uintptr_t)&time},
    {"gettimeofday", (uintptr_t)&gettimeofday},

    // Misc
    {"getenv", (uintptr_t)&ret0},
    {"srand", (uintptr_t)&srand},
    {"rand", (uintptr_t)&rand},
    {"exit", (uintptr_t)&exit},
    {"abort", (uintptr_t)&abort},
    {"sprintf", (uintptr_t)&sprintf},
    {"strncpy", (uintptr_t)&strncpy},
    {"strerror", (uintptr_t)&strerror},
    {"puts", (uintptr_t)&puts},
    {"__errno", (uintptr_t)&__errno},
    {"raise", (uintptr_t)&raise},
    {"__stack_chk_fail", (uintptr_t)&ret0},
    {"__cxa_finalize", (uintptr_t)&ret0},
    {"__cxa_begin_cleanup", (uintptr_t)&ret0},
    {"__cxa_pure_virtual", (uintptr_t)&ret0},
    {"__cxa_type_match", (uintptr_t)&ret0},
    {"__aeabi_atexit", (uintptr_t)&ret0},
    {"__gnu_Unwind_Find_exidx", (uintptr_t)&ret0},
    {"__assert2", (uintptr_t)&ret0},
    {"vprintf", (uintptr_t)&vprintf},

    // Pthread additions
    {"pthread_key_create", (uintptr_t)&pthread_key_create},
    {"pthread_getspecific", (uintptr_t)&pthread_getspecific},
    {"pthread_setspecific", (uintptr_t)&pthread_setspecific},
    {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait},
    {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast},

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
};

size_t default_dynlib_size = sizeof(default_dynlib) / sizeof(DynLibFunction);

// Debug helper to print unresolved symbols
void print_unresolved_symbols() {
    debugPrintf("Total symbols in default_dynlib: %d\n", default_dynlib_size);
}
