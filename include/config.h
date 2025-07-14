/*
 * config.h - Configuration system header for Fluffy Diver
 * Based on GTA SA Vita configuration methodology
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// Graphics quality levels
typedef enum {
    GRAPHICS_LOW = 0,
    GRAPHICS_MEDIUM = 1,
    GRAPHICS_HIGH = 2
} GraphicsQuality;

// FPS cap options
typedef enum {
    FPS_30 = 0,
    FPS_60 = 1,
    FPS_UNCAPPED = 2
} FpsCap;

// MSAA options
typedef enum {
    MSAA_OFF = 0,
    MSAA_2X = 1,
    MSAA_4X = 2
} MsaaLevel;

// Button layout options
typedef enum {
    LAYOUT_DEFAULT = 0,
    LAYOUT_LEFTHANDED = 1,
    LAYOUT_CUSTOM = 2
} ButtonLayout;

// VRAM usage options
typedef enum {
    VRAM_LOW = 0,
    VRAM_NORMAL = 1,
    VRAM_HIGH = 2
} VramUsage;

// Configuration structure
typedef struct {
    // Graphics settings
    int graphics_quality;
    int fps_cap;
    int msaa;
    int bilinear_filter;

    // Audio settings
    int master_volume;
    int sfx_volume;
    int music_volume;

    // Input settings
    int touch_controls;
    int gyroscope;
    int button_layout;

    // Performance settings
    int overclock;
    int gpu_overrides;
    int vram_usage;

    // Debug settings
    int debug_logging;
    int show_fps;
    int wireframe;
} FluffyDiverConfig;

// Global configuration instance
extern FluffyDiverConfig config;

// Configuration functions
void config_init(void);
int config_load(void);
int config_save(void);
void config_apply(void);

// Getter functions
int config_get_graphics_quality(void);
int config_get_fps_cap(void);
int config_get_msaa(void);
int config_get_bilinear_filter(void);
int config_get_master_volume(void);
int config_get_sfx_volume(void);
int config_get_music_volume(void);
int config_get_touch_controls(void);
int config_get_gyroscope(void);
int config_get_button_layout(void);
int config_get_overclock(void);
int config_get_gpu_overrides(void);
int config_get_vram_usage(void);
int config_get_debug_logging(void);
int config_get_show_fps(void);
int config_get_wireframe(void);

// Setter functions
void config_set_graphics_quality(int quality);
void config_set_fps_cap(int fps_cap);
void config_set_master_volume(int volume);
void config_set_debug_logging(int enable);

#endif // __CONFIG_H__
