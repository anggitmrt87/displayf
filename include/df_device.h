#ifndef DF_DEVICE_H
#define DF_DEVICE_H

#include <stdint.h>
#include "hw_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * df_device — struktur HAL DisplayFeature Xiaomi.
 *
 * LAYOUT INI BELUM DIKONFIRMASI. Setelah kita punya symbol dump dari
 * vendor .so asli, layout di bawah bisa disesuaikan.
 *
 * Untuk sekarang, kita definisikan sebagai opaque sehingga interposer
 * tidak mengakses field secara langsung (aman dari mismatch).
 */
typedef struct df_device df_device_t;

/* Function pointer signatures untuk HAL operations.
 * Ini yang akan kita intercept. Nama-nama diambil dari symbol
 * mangling yang kita temukan di strings analysis:
 *
 *   DisplayFeatureHal::setFeatureEnable(df_device*, int, int, int, int)
 *   DisplayFeatureHal::setFunctionEnable(df_device*, int, int, int, int)
 *   DisplayFeatureHal::getFunction(df_device*, int)
 *   DisplayFeatureHal::getCapabilities(df_device*, uint32_t*, int32_t*)
 *   DisplayFeatureHal::sendMessage(df_device*, int, int, std::string const&)
 *   DisplayFeatureHal::open(hw_module_t const*, char const*, hw_device_t**)
 *   DisplayFeatureHal::close(hw_device_t*)
 *   DisplayFeatureHal::init()
 *   DisplayFeatureHal::deinit()
 */

typedef int  (*df_set_feature_fn)(df_device_t*, int, int, int, int);
typedef int  (*df_set_function_fn)(df_device_t*, int, int, int, int);
typedef int  (*df_get_function_fn)(df_device_t*, int);
typedef int  (*df_get_caps_fn)(df_device_t*, uint32_t*, int32_t*);
typedef void (*df_send_message_fn)(df_device_t*, int, int, const void*);

/*
 * df_device — layout TIDAK DIPAKAI sampai kita punya symbol dump.
 * Interposer bekerja di level hw_device_t/hw_module_t saja.
 */

#ifdef __cplusplus
}
#endif

#endif /* DF_DEVICE_H */
