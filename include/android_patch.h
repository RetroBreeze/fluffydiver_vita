/*
 * android_patch.h - Android API bridge header for Fluffy Diver
 * Based on GTA SA Vita patch methodology - MINIMAL declarations to avoid conflicts
 * Reference: https://github.com/TheOfficialFloW/gtasa_vita/blob/master/loader/patch.h
 */

#ifndef __ANDROID_PATCH_H__
#define __ANDROID_PATCH_H__

#include <stdarg.h>
#include <stdint.h>

// ===== ANDROID LOG PRIORITIES =====
#define ANDROID_LOG_UNKNOWN 0
#define ANDROID_LOG_DEFAULT 1
#define ANDROID_LOG_VERBOSE 2
#define ANDROID_LOG_DEBUG   3
#define ANDROID_LOG_INFO    4
#define ANDROID_LOG_WARN    5
#define ANDROID_LOG_ERROR   6
#define ANDROID_LOG_FATAL   7
#define ANDROID_LOG_SILENT  8

// ===== ASSET MANAGER CONSTANTS =====
#define AASSET_MODE_UNKNOWN   0
#define AASSET_MODE_RANDOM    1
#define AASSET_MODE_STREAMING 2
#define AASSET_MODE_BUFFER    3

// ===== SCREEN ORIENTATION CONSTANTS =====
#define ANDROID_ORIENTATION_UNDEFINED  0
#define ANDROID_ORIENTATION_PORTRAIT   1
#define ANDROID_ORIENTATION_LANDSCAPE  2

// ===== BITMAP CONFIG CONSTANTS =====
#define ANDROID_BITMAP_FORMAT_NONE      0
#define ANDROID_BITMAP_FORMAT_RGBA_8888 1
#define ANDROID_BITMAP_FORMAT_RGB_565   4
#define ANDROID_BITMAP_FORMAT_RGBA_4444 7
#define ANDROID_BITMAP_FORMAT_A_8       8

// ===== MAIN INITIALIZATION FUNCTION =====
// This is the ONLY function declaration we need in this header
// All other Android functions are declared in so_util.h or implemented directly
void android_patch_init(void);

// ===== NOTE =====
// All other Android function declarations are in:
// - so_util.h (ALooper functions, Android API functions)
// - jni_patch.h (JNI types: JavaVM, JNIEnv)
// - default_dynlib.c (function implementations)
//
// This approach follows GTA SA Vita methodology of avoiding duplicate declarations

#endif // __ANDROID_PATCH_H__
