#include "hook_hal.h"
#include "symbol_resolver.h"

#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <unistd.h>       /* FIX: untuk close/read, walau tidak wajib di sini */

#define LOG_TAG "df-hook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace dfhook {

/* ===================================================================
 * Reference ke original open()/close() — di-set oleh interposer
 * =================================================================== */

static orig_open_fn  g_orig_open  = nullptr;
static orig_close_fn g_orig_close = nullptr;

void set_original_open(orig_open_fn fn) {
    g_orig_open = fn;
    LOGI("set_original_open(%p)", (void*)fn);
}

void set_original_close(orig_close_fn fn) {
    g_orig_close = fn;
    LOGI("set_original_close(%p)", (void*)fn);
}

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
 * =================================================================== */

void install_device_hook(hw_device_t* device) {
    if (!device) return;

    LOGI("install_device_hook(device=%p)", device);
    LOGI("  device->tag     = 0x%08x", device->tag);
    LOGI("  device->version = 0x%08x", device->version);
    LOGI("  device->module  = %p",     device->module);
    LOGI("  device->close   = %p",     device->close);

    /* Patch close() supaya kita bisa log saat device ditutup */
    if (device->close && g_orig_close == nullptr) {
        g_orig_close = device->close;
        device->close = hooked_close;
        LOGI("  -> close hooked");
    }

    /* TODO: setelah layout df_device diketahui dari symbol analysis,
     * patch function pointer setFeatureEnable di sini. */
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

    int ret = g_orig_open(module, id, device);
    LOGI("original open returned %d, *device=%p", ret,
         (device && *device) ? *device : nullptr);

    if (ret == 0 && device && *device) {
        install_device_hook(*device);
    }

    return ret;
}

} /* namespace dfhook */
