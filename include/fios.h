/*
 * fios.h - File I/O system header for Fluffy Diver
 * Based on GTA SA Vita FIOS methodology
 */

#ifndef __FIOS_H__
#define __FIOS_H__

#include <stdio.h>

// FIOS initialization and cleanup
int fios_init(void);
void fios_cleanup(void);

// Directory management
void fios_create_directories(void);
int fios_mkdir(const char *path);
int fios_list_directory(const char *path);

// Path translation
char *fios_translate_path(const char *path);
int fios_add_redirect(const char *from, const char *to);
void fios_print_redirects(void);

// File operations
int fios_file_exists(const char *path);
long fios_file_size(const char *path);
int fios_copy_file(const char *src, const char *dst);
int fios_remove(const char *path);

// Enhanced file operations
FILE *fios_fopen(const char *path, const char *mode);

// Android asset compatibility
int fios_asset_exists(const char *asset_path);
long fios_asset_size(const char *asset_path);
FILE *fios_asset_open(const char *asset_path, const char *mode);

// Device information
long long fios_get_free_space(const char *path);

#endif // __FIOS_H__
