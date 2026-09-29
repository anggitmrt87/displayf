#ifndef DISPLAYFEATURE_TYPES_H
#define DISPLAYFEATURE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================================================================
 * Error codes
 * =================================================================== */

typedef enum df_error {
    DF_OK                    =  0,
    DF_ERR_INVALID_ARG       = -1,
    DF_ERR_NOT_INITIALIZED   = -2,
    DF_ERR_SERVICE_NOT_FOUND = -3,
    DF_ERR_PERMISSION_DENIED = -4,
    DF_ERR_TRANSACTION_FAIL  = -5,
    DF_ERR_IO                = -6,
    DF_ERR_UNSUPPORTED       = -7,
    DF_ERR_TIMEOUT           = -8,
    DF_ERR_UNKNOWN           = -99,
} df_error_t;

/* ===================================================================
 * Feature case ID
 * =================================================================== */

typedef enum df_case {
    DF_CASE_UNKNOWN        = 0,
    DF_CASE_EYECARE        = 1,
    DF_CASE_NIGHT_MODE     = 2,
    DF_CASE_HBM            = 3,
    DF_CASE_CABC           = 4,
    DF_CASE_SRGB           = 5,
    DF_CASE_GAME_MODE      = 6,
    DF_CASE_VIDEO_MODE     = 7,
    DF_CASE_STANDARD       = 8,
    DF_CASE_HDR            = 9,
    DF_CASE_COLOR_TEMP     = 10,
} df_case_t;

/* ===================================================================
 * Reading mode type
 * =================================================================== */

typedef enum df_reading_mode {
    DF_READING_OFF        = 0,
    DF_READING_PAPER      = 1,
    DF_READING_SOFT       = 2,
    DF_READING_PAPER_SOFT = 3,
} df_reading_mode_t;

/* ===================================================================
 * Color scheme mode
 * =================================================================== */

typedef enum df_color_scheme {
    DF_COLOR_DEFAULT   = 0,
    DF_COLOR_VIVID     = 1,
    DF_COLOR_SRGB      = 2,
    DF_COLOR_WARM      = 3,
    DF_COLOR_COOL      = 4,
} df_color_scheme_t;

/* ===================================================================
 * Backend
 * =================================================================== */

typedef enum df_backend {
    DF_BACKEND_AUTO     = 0,
    DF_BACKEND_HIDL     = 1,
    DF_BACKEND_SERVICE  = 2,
    DF_BACKEND_SYSFS    = 3,
} df_backend_t;

/* ===================================================================
 * State snapshot
 * =================================================================== */

typedef struct df_state {
    /* Reading mode */
    int  reading_mode_enabled;
    int  reading_mode_type;
    int  reading_mode_ct_level;

    /* Color scheme */
    int  color_scheme_mode_type;
    int  color_scheme_ct_level;

    /* HBM */
    int  hbm_enabled;

    /* Game */
    int  game_hdr_enabled;

    /* Flags */
    int  force_disable_eye_care;
    int  auto_adjust_enable;
    int  paper_color_type;

    /* Backlight */
    int  backlight_current;
    int  backlight_max;

    /* Device info */
    char codename[32];
    char panel[128];
} df_state_t;

/* ===================================================================
 * Opaque client handle
 * =================================================================== */

typedef struct df_client df_client_t;

/* ===================================================================
 * Transaction codes untuk framework service "displayfeature"
 * (hasil probe v3)
 * =================================================================== */

enum df_tx {
    DF_TX_GET_CAPABILITIES             = 1,
    DF_TX_REGISTER_READ_APP_LISTENER   = 2,
    DF_TX_SET_EYECARE_SWITCH           = 3,
    DF_TX_GET_READ_APP_LIST            = 4,
    DF_TX_GET_GAME_APP_LIST            = 5,
    DF_TX_UNREG_READ_APP_LISTENER      = 6,
    DF_TX_REGISTER_DF_CALLBACK         = 7,
    DF_TX_UNREGISTER_DF_CALLBACK       = 8,
    DF_TX_GET_ALL_EFFECT_SCOPE_APPS    = 9,
    DF_TX_10                           = 10,
};

/* Alias tanpa prefix DF_ untuk kompatibilitas */
#define TX_GET_CAPABILITIES             DF_TX_GET_CAPABILITIES
#define TX_REGISTER_READ_APP_LISTENER   DF_TX_REGISTER_READ_APP_LISTENER
#define TX_SET_EYECARE_SWITCH           DF_TX_SET_EYECARE_SWITCH
#define TX_GET_READ_APP_LIST            DF_TX_GET_READ_APP_LIST
#define TX_GET_GAME_APP_LIST            DF_TX_GET_GAME_APP_LIST
#define TX_UNREG_READ_APP_LISTENER      DF_TX_UNREG_READ_APP_LISTENER
#define TX_REGISTER_DF_CALLBACK         DF_TX_REGISTER_DF_CALLBACK
#define TX_UNREGISTER_DF_CALLBACK       DF_TX_UNREGISTER_DF_CALLBACK
#define TX_GET_ALL_EFFECT_SCOPE_APPS    DF_TX_GET_ALL_EFFECT_SCOPE_APPS

/* ===================================================================
 * Path sysfs
 * =================================================================== */

#ifndef DF_BACKLIGHT_PATH
#define DF_BACKLIGHT_PATH \
    "/sys/class/backlight/panel0-backlight/brightness"
#endif

#ifndef DF_MAX_BRIGHTNESS_PATH
#define DF_MAX_BRIGHTNESS_PATH \
    "/sys/class/backlight/panel0-backlight/max_brightness"
#endif

#ifndef DF_PANEL_INFO_PATH
#define DF_PANEL_INFO_PATH \
    "/sys/class/drm/card0-DSI-1/panel_info"
#endif

/* Alias supaya kompatibel dengan client.cpp */
#define BACKLIGHT_PATH       DF_BACKLIGHT_PATH
#define MAX_BRIGHTNESS_PATH  DF_MAX_BRIGHTNESS_PATH

/* ===================================================================
 * Service names
 * =================================================================== */

#ifndef DF_SERVICE_NAME
#define DF_SERVICE_NAME "displayfeature"
#endif

#ifndef DF_SERVICE_INTERFACE
#define DF_SERVICE_INTERFACE \
    "miui.hardware.display.IDisplayFeatureManager"
#endif

#ifndef DF_HIDL_INTERFACE
#define DF_HIDL_INTERFACE \
    "vendor.xiaomi.hardware.displayfeature@1.0::IDisplayFeature"
#endif

/* Alias tanpa DF_ */
#define SERVICE_NAME       DF_SERVICE_NAME
#define SERVICE_INTERFACE  DF_SERVICE_INTERFACE
#define HIDL_INTERFACE     DF_HIDL_INTERFACE

/* ===================================================================
 * Device codename
 * =================================================================== */

#define DF_DEVICE_LIME    "lime"
#define DF_DEVICE_LEMON   "lemon"
#define DF_DEVICE_POMELO  "pomelo"
#define DF_DEVICE_CITRUS  "citrus"

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISPLAYFEATURE_TYPES_H */
