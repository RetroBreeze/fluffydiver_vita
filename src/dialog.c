/*
 * dialog.c - Error dialog system for Fluffy Diver
 * Based on GTA SA Vita dialog methodology
 * Reference: https://github.com/TheOfficialFloW/conduit_vita/blob/master/loader/dialog.c
 */

#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "dialog.h"

// Dialog states
static int dialog_initialized = 0;
static SceCommonDialogStatus dialog_status = SCE_COMMON_DIALOG_STATUS_NONE;

// Initialize dialog system
int dialog_init(void) {
    if (dialog_initialized) {
        return 1; // Already initialized
    }

    SceCommonDialogConfigParam config;
    sceCommonDialogConfigParamInit(&config);

    int ret = sceCommonDialogSetConfigParam(&config);
    if (ret < 0) {
        printf("Dialog: Failed to initialize config param: 0x%08X\n", ret);
        return 0;
    }

    dialog_initialized = 1;
    printf("Dialog: System initialized\n");
    return 1;
}

// Show error dialog
void dialog_error(const char *fmt, ...) {
    if (!dialog_initialized) {
        if (!dialog_init()) {
            printf("Dialog: Cannot show error dialog - init failed\n");
            return;
        }
    }

    // Format the error message
    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    printf("Dialog: ERROR - %s\n", message);

    // Show message dialog
    SceMsgDialogParam param;
    sceMsgDialogParamInit(&param);

    param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
    param.userMsgParam.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
    param.userMsgParam.msg = message;

    int ret = sceMsgDialogInit(&param);
    if (ret < 0) {
        printf("Dialog: Failed to initialize error dialog: 0x%08X\n", ret);
        return;
    }

    // Wait for dialog to complete
    while (1) {
        dialog_status = sceMsgDialogGetStatus();

        if (dialog_status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            break;
        }

        sceKernelDelayThread(10000); // 10ms
    }

    sceMsgDialogTerm();
    printf("Dialog: Error dialog closed\n");
}

// Show warning dialog
void dialog_warning(const char *fmt, ...) {
    if (!dialog_initialized) {
        if (!dialog_init()) {
            printf("Dialog: Cannot show warning dialog - init failed\n");
            return;
        }
    }

    // Format the warning message
    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    printf("Dialog: WARNING - %s\n", message);

    // Prepend "Warning: " to the message
    char full_message[600];
    snprintf(full_message, sizeof(full_message), "Warning: %s", message);

    // Show message dialog
    SceMsgDialogParam param;
    sceMsgDialogParamInit(&param);

    param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
    param.userMsgParam.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
    param.userMsgParam.msg = full_message;

    int ret = sceMsgDialogInit(&param);
    if (ret < 0) {
        printf("Dialog: Failed to initialize warning dialog: 0x%08X\n", ret);
        return;
    }

    // Wait for dialog to complete
    while (1) {
        dialog_status = sceMsgDialogGetStatus();

        if (dialog_status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            break;
        }

        sceKernelDelayThread(10000); // 10ms
    }

    sceMsgDialogTerm();
    printf("Dialog: Warning dialog closed\n");
}

// Show info dialog
void dialog_info(const char *fmt, ...) {
    if (!dialog_initialized) {
        if (!dialog_init()) {
            printf("Dialog: Cannot show info dialog - init failed\n");
            return;
        }
    }

    // Format the info message
    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    printf("Dialog: INFO - %s\n", message);

    // Show message dialog
    SceMsgDialogParam param;
    sceMsgDialogParamInit(&param);

    param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
    param.userMsgParam.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
    param.userMsgParam.msg = message;

    int ret = sceMsgDialogInit(&param);
    if (ret < 0) {
        printf("Dialog: Failed to initialize info dialog: 0x%08X\n", ret);
        return;
    }

    // Wait for dialog to complete
    while (1) {
        dialog_status = sceMsgDialogGetStatus();

        if (dialog_status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            break;
        }

        sceKernelDelayThread(10000); // 10ms
    }

    sceMsgDialogTerm();
    printf("Dialog: Info dialog closed\n");
}

// Show yes/no confirmation dialog
int dialog_confirm(const char *fmt, ...) {
    if (!dialog_initialized) {
        if (!dialog_init()) {
            printf("Dialog: Cannot show confirm dialog - init failed\n");
            return 0; // Default to "No"
        }
    }

    // Format the confirmation message
    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    printf("Dialog: CONFIRM - %s\n", message);

    // Show message dialog
    SceMsgDialogParam param;
    sceMsgDialogParamInit(&param);

    param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
    param.userMsgParam.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_YESNO;
    param.userMsgParam.msg = message;

    int ret = sceMsgDialogInit(&param);
    if (ret < 0) {
        printf("Dialog: Failed to initialize confirm dialog: 0x%08X\n", ret);
        return 0;
    }

    // Wait for dialog to complete
    while (1) {
        dialog_status = sceMsgDialogGetStatus();

        if (dialog_status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            break;
        }

        sceKernelDelayThread(10000); // 10ms
    }

    // Get the result
    SceMsgDialogResult result;
    sceMsgDialogGetResult(&result);

    sceMsgDialogTerm();

    int user_choice = (result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_YES) ? 1 : 0;
    printf("Dialog: Confirm dialog closed, user chose: %s\n", user_choice ? "Yes" : "No");

    return user_choice;
}

// Show progress dialog (for loading)
static SceMsgDialogProgressBarParam progress_param;
static int progress_dialog_active = 0;

int dialog_progress_start(const char *message) {
    if (!dialog_initialized) {
        if (!dialog_init()) {
            printf("Dialog: Cannot show progress dialog - init failed\n");
            return 0;
        }
    }

    if (progress_dialog_active) {
        printf("Dialog: Progress dialog already active\n");
        return 0;
    }

    printf("Dialog: Starting progress dialog: %s\n", message);

    SceMsgDialogParam param;
    sceMsgDialogParamInit(&param);

    param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
    param.progBarParam.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;
    param.progBarParam.msg = message;

    int ret = sceMsgDialogInit(&param);
    if (ret < 0) {
        printf("Dialog: Failed to initialize progress dialog: 0x%08X\n", ret);
        return 0;
    }

    progress_dialog_active = 1;
    return 1;
}

void dialog_progress_update(int percentage) {
    if (!progress_dialog_active) {
        return;
    }

    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    sceMsgDialogProgressBarSetValue(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, percentage);
}

void dialog_progress_end(void) {
    if (!progress_dialog_active) {
        return;
    }

    // Wait for dialog to complete
    while (1) {
        dialog_status = sceMsgDialogGetStatus();

        if (dialog_status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            break;
        }

        sceKernelDelayThread(10000); // 10ms
    }

    sceMsgDialogTerm();
    progress_dialog_active = 0;
    printf("Dialog: Progress dialog ended\n");
}

// Check if any dialog is currently active
int dialog_is_active(void) {
    if (!dialog_initialized) {
        return 0;
    }

    dialog_status = sceMsgDialogGetStatus();
    return (dialog_status == SCE_COMMON_DIALOG_STATUS_RUNNING);
}

// Cleanup dialog system
void dialog_cleanup(void) {
    if (progress_dialog_active) {
        dialog_progress_end();
    }

    dialog_initialized = 0;
    printf("Dialog: System cleaned up\n");
}
