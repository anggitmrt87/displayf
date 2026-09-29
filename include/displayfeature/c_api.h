#ifndef DISPLAYFEATURE_C_API_H
#define DISPLAYFEATURE_C_API_H

#include <stdint.h>
#include <stdbool.h>
#include "displayfeature/types.h"
#include "displayfeature/version.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===================================================================
 * Lifecycle
 * =================================================================== */

/* Buat client baru. Return NULL jika gagal alokasi. */
df_client_t* df_client_create(void);

/* Buat client dengan backend tertentu. */
df_client_t* df_client_create_with_backend(df_backend_t backend);

/* Buka client (siapkan koneksi ke service). */
df_error_t df_client_open(df_client_t* c);

/* Tutup dan bebaskan. NULL-safe. */
void df_client_destroy(df_client_t* c);

/* ===================================================================
 * Info
 * =================================================================== */

/* 1 jika device didukung (lime/lemon/pomelo/citrus). */
int df_is_supported_device(char* out_codename, size_t len);

/* Copy codename device ke buffer. Return DF_OK / DF_ERR_INVALID_ARG. */
df_error_t df_device_codename(df_client_t* c, char* buf, size_t len);

/* Copy panel name ke buffer. */
df_error_t df_panel_name(df_client_t* c, char* buf, size_t len);

/* 1 jika proses saat ini root. */
int df_is_root(void);

/* Backend yang sedang aktif. */
df_backend_t df_active_backend(df_client_t* c);

/* ===================================================================
 * State
 * =================================================================== */

/* Ambil snapshot state. */
df_error_t df_get_state(df_client_t* c, df_state_t* out);

/* Ambil dumpsys lengkap. Caller harus free() string yang dikembalikan. */
df_error_t df_dumpsys(df_client_t* c, char** out_str);
void      df_free_string(char* s);

/* ===================================================================
 * Features
 * =================================================================== */

df_error_t df_set_eyecare(df_client_t* c, int value);
df_error_t df_set_reading_mode(df_client_t* c, df_reading_mode_t mode);
df_error_t df_set_reading_mode_ct(df_client_t* c, int level);
df_error_t df_set_color_scheme(df_client_t* c, df_color_scheme_t scheme);
df_error_t df_set_color_scheme_ct(df_client_t* c, int level);
df_error_t df_set_hbm(df_client_t* c, int on);
df_error_t df_set_cabc(df_client_t* c, int mode);

/* ===================================================================
 * Backlight
 * =================================================================== */

df_error_t df_get_backlight(df_client_t* c, int* out_value);
df_error_t df_get_backlight_max(df_client_t* c, int* out_value);
df_error_t df_set_backlight(df_client_t* c, int value);

/* ===================================================================
 * Generic transaction
 * =================================================================== */

/* Panggil service.transact(tx_code, [args...]). */
df_error_t df_call(df_client_t* c, int tx_code,
                   const int32_t* args, size_t nargs,
                   int* out_result);

/* Panggil HIDL setFeature(display_id, case_id, mode_id, cookie). */
df_error_t df_hidl_set_feature(df_client_t* c,
                               int display_id, int case_id,
                               int mode_id, int cookie,
                               int* out_result);

/* ===================================================================
 * Probing
 * =================================================================== */

typedef struct df_probe_entry {
    int          code;
    int          arg_mismatch;
    int          perm_denied;
    int          ok;
    const char*  raw;   /* owned by array; free via df_probe_free */
} df_probe_entry_t;

/* Probe transaction code range. Caller harus panggil df_probe_free(). */
df_error_t df_probe(df_client_t* c, int start, int end,
                    df_probe_entry_t** out_entries, size_t* out_count);

/* Bebaskan hasil df_probe. */
void df_probe_free(df_probe_entry_t* entries, size_t count);

/* ===================================================================
 * Error string
 * =================================================================== */

const char* df_error_string(df_error_t err);

/* ===================================================================
 * One-shot convenience
 * =================================================================== */

df_error_t df_oneshot_set_reading_mode(df_reading_mode_t mode);
df_error_t df_oneshot_set_backlight(int value);
df_error_t df_oneshot_state(df_state_t* out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISPLAYFEATURE_C_API_H */
