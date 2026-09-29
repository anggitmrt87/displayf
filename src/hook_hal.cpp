#include "hook_hal.h"
#include "symbol_resolver.h"

#include <android/log.h>
#include <dlfcn.h>
#include <cstring>

#define LOG_TAG "df-hook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace dfhook {

/* ===================================================================
 * Reference ke original open() — di-set oleh interposer
 * =================================================================== */

typedef int (*orig_open_fn)(const hw_module_t*, const char*, hw_device_t**);
typedef int (*orig_close_fn)(hw_device_t*);

static orig_open_fn  g_orig_open  = nullptr;
static orig_close_fn g_orig_close = nullptr;

void set_original_open(orig_open_fn fn)  { g_orig_open  = fn; }
void set_original_close(orig_close_fn fn) { g_orig_close = fn; }

/* ===================================================================
 * Hooked close
 * =================================================================== */

int hooked_close(hw_device_t* device) {
    LOGI("hooked_close(device=%p)", device);

    if (!g_orig_close) {
        LOGW("original close not available");
        return -1;
    }
    return g_orig_close(device);
}

/* ===================================================================
 * Install hook pada device
 *
 * Karena kita belum tahu layout df_device persis, versi ini hanya
 * log & observasi. Setelah symbol analysis selesai, function ini
 * bisa dimodifikasi untuk patch function pointer di dalam device.
 * =================================================================== */

void install_device_hook(hw_device_t* device) {
    if (!device) return;

    LOGI("install_device_hook(device=%p)", device);
    LOGI("  device->tag     = 0x%08x", device->tag);
    LOGI("  device->version = 0x%08x", device->version);
    LOGI("  device->module  = %p", device->module);
    LOGI("  device->close   = %p", device->close);

    /* Patch close() supaya kita bisa log saat device ditutup */
    if (device->close && g_orig_close == nullptr) {
        /* Simpan original */
        g_orig_close = device->close;
        /* Ganti dengan hook kita */
        device->close = hooked_close;
        LOGI("  -> close hooked");
    }

    /* TODO: setelah kita tahu offset setFeatureEnable di dalam
     * struct df_device, patch di sini. Contoh (pseudo):
     *
     *   struct df_device_layout {
     *       hw_device_t common;
     *       int (*set_feature_enable)(...);
     *       int (*set_function_enable)(...);
     *       ...
     *   };
     *   auto* df = reinterpret_cast<df_device_layout*>(device);
     *   g_orig_set_feature = df->set_feature_enable;
     *   df->set_feature_enable = hooked_set_feature;
     */
}

/* ===================================================================
 * Hooked open
 * =================================================================== */

int hooked_open(const hw_module_t* module, const char* id,
                hw_device_t** device) {
    LOGI("hooked_open(module=%p, id=%s, device=%p)", module,
         id ? id : "(null)", device);

    if (!g_orig_open) {
        LOGE("original open not available");
        return -1;
    }

    /* Panggil original */
    int ret = g_orig_open(module, id, device);
    LOGI("original open returned %d, *device=%p", ret,
         (device && *device) ? *device : nullptr);

    if (ret == 0 && device && *device) {
        install_device_hook(*device);
    }

    return ret;
}

} /* namespace dfhook */
