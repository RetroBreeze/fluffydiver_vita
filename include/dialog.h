/*
 * dialog.h - Dialog system header for Fluffy Diver
 * Based on GTA SA Vita dialog methodology
 */

#ifndef __DIALOG_H__
#define __DIALOG_H__

// Dialog initialization
int dialog_init(void);

// Basic dialog functions
void dialog_error(const char *fmt, ...);
void dialog_warning(const char *fmt, ...);
void dialog_info(const char *fmt, ...);
int dialog_confirm(const char *fmt, ...);

// Progress dialog functions
int dialog_progress_start(const char *message);
void dialog_progress_update(int percentage);
void dialog_progress_end(void);

// Utility functions
int dialog_is_active(void);
void dialog_cleanup(void);

#endif // __DIALOG_H__
