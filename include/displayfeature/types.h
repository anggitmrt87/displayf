#ifndef DISPLAYFEATURE_TYPES_H
#define DISPLAYFEATURE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

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
 * Feature case ID (dari analysis strings DisplayFeatureHal.cpp)
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
    DF_BACKEND_AUTO     = 0,   /* auto-detect */
    DF_BACKEND_HIDL     = 1,   /* direct HIDL (butuh libbinder) */
    DF_BACKEND_SERVICE  = 2,   /* via `service call` shell */
    DF_BACKEND_SYSFS    = 3,   /* direct sysfs (backlight only) */
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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISPLAYFEATURE_TYPES_H */
