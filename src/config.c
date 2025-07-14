/*
 * config.c - Configuration system for Fluffy Diver
 * Based on GTA SA Vita configuration methodology
 * Reference: https://github.com/TheOfficialFloW/conduit_vita/blob/master/loader/config.c
 */

#include <vitasdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

// Configuration file path
#define CONFIG_FILE_PATH "ux0:data/fluffydiver/config.txt"

// Global configuration
FluffyDiverConfig config;

// Default configuration values
static void config_set_defaults(void) {
    // Graphics settings
    config.graphics_quality = GRAPHICS_HIGH;
    config.fps_cap = FPS_60;
    config.msaa = MSAA_4X;
    config.bilinear_filter = 1;

    // Audio settings
    config.master_volume = 100;
    config.sfx_volume = 100;
    config.music_volume = 100;

    // Input settings
    config.touch_controls = 1;
    config.gyroscope = 0;
    config.button_layout = LAYOUT_DEFAULT;

    // Performance settings
    config.overclock = 0;
    config.gpu_overrides = 0;
    config.vram_usage = VRAM_NORMAL;

    // Debug settings
    config.debug_logging = 1;
    config.show_fps = 0;
    config.wireframe = 0;

    printf("Configuration: Set to defaults\n");
}

static int config_parse_line(const char *line) {
    char key[64];
    char value[64];

    if (sscanf(line, "%63s = %63s", key, value) != 2) {
        return 0; // Skip invalid lines
    }

    // Graphics settings
    if (strcmp(key, "graphics_quality") == 0) {
        if (strcmp(value, "low") == 0) config.graphics_quality = GRAPHICS_LOW;
        else if (strcmp(value, "medium") == 0) config.graphics_quality = GRAPHICS_MEDIUM;
        else if (strcmp(value, "high") == 0) config.graphics_quality = GRAPHICS_HIGH;
    }
    else if (strcmp(key, "fps_cap") == 0) {
        if (strcmp(value, "30") == 0) config.fps_cap = FPS_30;
        else if (strcmp(value, "60") == 0) config.fps_cap = FPS_60;
        else if (strcmp(value, "uncapped") == 0) config.fps_cap = FPS_UNCAPPED;
    }
    else if (strcmp(key, "msaa") == 0) {
        if (strcmp(value, "off") == 0) config.msaa = MSAA_OFF;
        else if (strcmp(value, "2x") == 0) config.msaa = MSAA_2X;
        else if (strcmp(value, "4x") == 0) config.msaa = MSAA_4X;
    }
    else if (strcmp(key, "bilinear_filter") == 0) {
        config.bilinear_filter = atoi(value);
    }

    // Audio settings
    else if (strcmp(key, "master_volume") == 0) {
        config.master_volume = atoi(value);
        if (config.master_volume > 100) config.master_volume = 100;
        if (config.master_volume < 0) config.master_volume = 0;
    }
    else if (strcmp(key, "sfx_volume") == 0) {
        config.sfx_volume = atoi(value);
        if (config.sfx_volume > 100) config.sfx_volume = 100;
        if (config.sfx_volume < 0) config.sfx_volume = 0;
    }
    else if (strcmp(key, "music_volume") == 0) {
        config.music_volume = atoi(value);
        if (config.music_volume > 100) config.music_volume = 100;
        if (config.music_volume < 0) config.music_volume = 0;
    }

    // Input settings
    else if (strcmp(key, "touch_controls") == 0) {
        config.touch_controls = atoi(value);
    }
    else if (strcmp(key, "gyroscope") == 0) {
        config.gyroscope = atoi(value);
    }
    else if (strcmp(key, "button_layout") == 0) {
        if (strcmp(value, "default") == 0) config.button_layout = LAYOUT_DEFAULT;
        else if (strcmp(value, "lefthanded") == 0) config.button_layout = LAYOUT_LEFTHANDED;
        else if (strcmp(value, "custom") == 0) config.button_layout = LAYOUT_CUSTOM;
    }

    // Performance settings
    else if (strcmp(key, "overclock") == 0) {
        config.overclock = atoi(value);
    }
    else if (strcmp(key, "gpu_overrides") == 0) {
        config.gpu_overrides = atoi(value);
    }
    else if (strcmp(key, "vram_usage") == 0) {
        if (strcmp(value, "low") == 0) config.vram_usage = VRAM_LOW;
        else if (strcmp(value, "normal") == 0) config.vram_usage = VRAM_NORMAL;
        else if (strcmp(value, "high") == 0) config.vram_usage = VRAM_HIGH;
    }

    // Debug settings
    else if (strcmp(key, "debug_logging") == 0) {
        config.debug_logging = atoi(value);
    }
    else if (strcmp(key, "show_fps") == 0) {
        config.show_fps = atoi(value);
    }
    else if (strcmp(key, "wireframe") == 0) {
        config.wireframe = atoi(value);
    }

    return 1;
}

int config_load(void) {
    FILE *file = fopen(CONFIG_FILE_PATH, "r");
    if (!file) {
        printf("Configuration: File not found, using defaults\n");
        return 0; // Not an error, just use defaults
    }

    char line[128];
    int lines_parsed = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\r\n")] = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (config_parse_line(line)) {
            lines_parsed++;
        }
    }

    fclose(file);
    printf("Configuration: Loaded %d settings from config file\n", lines_parsed);
    return 1;
}

int config_save(void) {
    FILE *file = fopen(CONFIG_FILE_PATH, "w");
    if (!file) {
        printf("Configuration: ERROR - Could not create config file\n");
        return 0;
    }

    fprintf(file, "# Fluffy Diver PS Vita Configuration\n");
    fprintf(file, "# Generated automatically - edit with care\n\n");

    // Graphics settings
    fprintf(file, "[Graphics]\n");
    fprintf(file, "graphics_quality = %s\n",
            config.graphics_quality == GRAPHICS_LOW ? "low" :
            config.graphics_quality == GRAPHICS_MEDIUM ? "medium" : "high");
    fprintf(file, "fps_cap = %s\n",
            config.fps_cap == FPS_30 ? "30" :
            config.fps_cap == FPS_60 ? "60" : "uncapped");
    fprintf(file, "msaa = %s\n",
            config.msaa == MSAA_OFF ? "off" :
            config.msaa == MSAA_2X ? "2x" : "4x");
    fprintf(file, "bilinear_filter = %d\n", config.bilinear_filter);
    fprintf(file, "\n");

    // Audio settings
    fprintf(file, "[Audio]\n");
    fprintf(file, "master_volume = %d\n", config.master_volume);
    fprintf(file, "sfx_volume = %d\n", config.sfx_volume);
    fprintf(file, "music_volume = %d\n", config.music_volume);
    fprintf(file, "\n");

    // Input settings
    fprintf(file, "[Input]\n");
    fprintf(file, "touch_controls = %d\n", config.touch_controls);
    fprintf(file, "gyroscope = %d\n", config.gyroscope);
    fprintf(file, "button_layout = %s\n",
            config.button_layout == LAYOUT_DEFAULT ? "default" :
            config.button_layout == LAYOUT_LEFTHANDED ? "lefthanded" : "custom");
    fprintf(file, "\n");

    // Performance settings
    fprintf(file, "[Performance]\n");
    fprintf(file, "overclock = %d\n", config.overclock);
    fprintf(file, "gpu_overrides = %d\n", config.gpu_overrides);
    fprintf(file, "vram_usage = %s\n",
            config.vram_usage == VRAM_LOW ? "low" :
            config.vram_usage == VRAM_NORMAL ? "normal" : "high");
    fprintf(file, "\n");

    // Debug settings
    fprintf(file, "[Debug]\n");
    fprintf(file, "debug_logging = %d\n", config.debug_logging);
    fprintf(file, "show_fps = %d\n", config.show_fps);
    fprintf(file, "wireframe = %d\n", config.wireframe);

    fclose(file);
    printf("Configuration: Saved to config file\n");
    return 1;
}

void config_init(void) {
    printf("Configuration: Initializing...\n");

    // Set defaults first
    config_set_defaults();

    // Try to load from file (overwrites defaults)
    config_load();

    // Apply configuration settings
    config_apply();

    printf("Configuration: Initialization complete\n");
}

void config_apply(void) {
    printf("Configuration: Applying settings...\n");

    // Apply graphics settings
    if (config.overclock) {
        printf("Configuration: Overclocking enabled\n");
        // Could use PSVshell integration here
    }

    // Apply MSAA setting to VitaGL
    // This would need to be done during VitaGL initialization

    // Apply performance settings
    if (config.vram_usage == VRAM_HIGH) {
        printf("Configuration: High VRAM usage mode\n");
    }

    printf("Configuration: Settings applied\n");
}

// Getter functions for other modules
int config_get_graphics_quality(void) {
    return config.graphics_quality;
}

int config_get_fps_cap(void) {
    return config.fps_cap;
}

int config_get_msaa(void) {
    return config.msaa;
}

int config_get_bilinear_filter(void) {
    return config.bilinear_filter;
}

int config_get_master_volume(void) {
    return config.master_volume;
}

int config_get_sfx_volume(void) {
    return config.sfx_volume;
}

int config_get_music_volume(void) {
    return config.music_volume;
}

int config_get_touch_controls(void) {
    return config.touch_controls;
}

int config_get_gyroscope(void) {
    return config.gyroscope;
}

int config_get_button_layout(void) {
    return config.button_layout;
}

int config_get_overclock(void) {
    return config.overclock;
}

int config_get_gpu_overrides(void) {
    return config.gpu_overrides;
}

int config_get_vram_usage(void) {
    return config.vram_usage;
}

int config_get_debug_logging(void) {
    return config.debug_logging;
}

int config_get_show_fps(void) {
    return config.show_fps;
}

int config_get_wireframe(void) {
    return config.wireframe;
}

// Setter functions for runtime changes
void config_set_graphics_quality(int quality) {
    config.graphics_quality = quality;
}

void config_set_fps_cap(int fps_cap) {
    config.fps_cap = fps_cap;
}

void config_set_master_volume(int volume) {
    if (volume >= 0 && volume <= 100) {
        config.master_volume = volume;
    }
}

void config_set_debug_logging(int enable) {
    config.debug_logging = enable;
}
