/*
 * interposer.cpp
 *
 * Entry point .so interposer untuk displayfeature.default.so.
 *
 * Cara kerja:
 *   1. File asli di-rename jadi displayfeature.default.so.orig
 *   2. .so ini ditaruh di /vendor/lib64/hw/displayfeature.default.so
 *   3. Saat sistem load .so ini, constructor akan dlopen file .orig
 *   4. Export HAL_MODULE_INFO_SYM (HMI) dengan method table kita
 *   5. Setiap panggilan open() di-intercept, lalu diteruskan ke original
 *   6. Device yang dikembalikan bisa dipatch (function pointer-nya)
 */

#define LOG_TAG "df-hook"

#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <cstring>

#include "hw_module.h"
#include "symbol_resolver.h"
#include "hook_hal.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace dfhook;

/* ===================================================================
 * Nama file original (backup)
 * =================================================================== */

static const char* ORIG_PATH_64 =
    "/vendor/lib64/hw/displayfeature.default.so.orig";
static const char* ORIG_PATH_32 =
    "/vendor/lib/hw/displayfeature.default.so.orig";

static const char* ORIG_PATH_FALLBACK_64 =
    "/data/adb/modules/displayfeature_hook/system/vendor/lib64/hw/displayfeature.default.so.orig";
static const char* ORIG_PATH_FALLBACK_32 =
    "/data/adb/modules/displayfeature_hook/system/vendor/lib/hw/displayfeature.default.so.orig";

/* ===================================================================
 * State global
 * =================================================================== */

struct OrigHandle {
    void* dl = nullptr;
    hw_module_t* module = nullptr;
    hw_module_methods_t* methods = nullptr;
    int (*open)(const hw_module_t*, const char*, hw_device_t**) = nullptr;
};

static OrigHandle g_orig;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;
static bool g_init_done = false;

/* ===================================================================
 * Cari path original yang benar
 * =================================================================== */

static const char* find_orig_path() {
    /* Coba semua kandidat, pakai yang pertama bisa diakses */
    const char* candidates[] = {
        ORIG_PATH_64,
        ORIG_PATH_FALLBACK_64,
        ORIG_PATH_32,
        ORIG_PATH_FALLBACK_32,
        nullptr
    };

    for (int i = 0; candidates[i]; i++) {
        if (access(candidates[i], R_OK) == 0) {
            LOGI("found original at: %s", candidates[i]);
            return candidates[i];
        }
    }
    return nullptr;
}

/* ===================================================================
 * Load original .so
 * =================================================================== */

static void load_orig() {
    const char* path = find_orig_path();
    if (!path) {
        LOGE("no original .so found in any candidate path");
        return;
    }

    LOGI("loading original: %s", path);
    g_orig.dl = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!g_orig.dl) {
        LOGE("dlopen failed: %s", dlerror());
        return;
    }

    /* Ambil HAL_MODULE_INFO_SYM — biasanya symbol "HMI" */
    const char* mod_syms[] = {
        "HMI",
        "HAL_MODULE_INFO_SYM",
        "hmi",
        "_ZL3HMIPK11hw_module_t",   /* C++ mangled, internal linkage */
        nullptr
    };
    g_orig.module = (hw_module_t*)resolve_first(g_orig.dl, mod_syms, 4);
    if (!g_orig.module) {
        LOGE("HAL_MODULE_INFO_SYM not found in original");
        dlclose(g_orig.dl);
        g_orig.dl = nullptr;
        return;
    }

    g_orig.methods = g_orig.module->methods;
    if (g_orig.methods) {
        g_orig.open = g_orig.methods->open;
    }

    LOGI("original loaded:");
    LOGI("  module   = %p", g_orig.module);
    LOGI("  methods  = %p", g_orig.methods);
    LOGI("  open     = %p", g_orig.open);
    LOGI("  id       = %s", g_orig.module->id    ? g_orig.module->id    : "(null)");
    LOGI("  name     = %s", g_orig.module->name  ? g_orig.module->name  : "(null)");
    LOGI("  author   = %s", g_orig.module->author? g_orig.module->author: "(null)");

    /* Beritahu hook_hal tentang original open */
    set_original_open(g_orig.open);
}

static void ensure_orig_loaded() {
    pthread_once(&g_once, load_orig);
    g_init_done = true;
}

/* ===================================================================
 * Hooked open (delegate ke hook_hal)
 * =================================================================== */

extern "C" int df_hooked_open(const hw_module_t* module, const char* id,
                              hw_device_t** device) {
    ensure_orig_loaded();
    return hooked_open(module, id, device);
}

/* ===================================================================
 * Export HAL_MODULE_INFO_SYM
 *
 * Ini adalah symbol yang dibaca hw_get_module() untuk menemukan module.
 * Kita export dengan nama yang sama persis.
 * =================================================================== */

extern "C" {

static hw_module_methods_t g_our_methods = {
    .open = df_hooked_open,
};

__attribute__((visibility("default"), used))
hw_module_t HMI = {
    .tag                = HARDWARE_MODULE_TAG,
    .module_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
    .hal_api_version    = HARDWARE_MAKE_API_VERSION(0, 0),
    .id                 = "displayfeature",
    .name               = "DisplayFeature HAL (interposer)",
    .author             = "anggitmrt87",
    .methods            = &g_our_methods,
    .dso                = nullptr,
    .reserved           = {0},
};

/* Alias: beberapa versi Android mencari dengan nama ini */
__attribute__((visibility("default"), used))
hw_module_t* HAL_MODULE_INFO_SYM = &HMI;

/* ===================================================================
 * Constructor & destructor
 * =================================================================== */

__attribute__((constructor))
static void on_load() {
    LOGI("==============================================");
    LOGI("DisplayFeature interposer loaded");
    LOGI("  build   : " __DATE__ " " __TIME__);
    LOGI("  version : 1.0.0");
    LOGI("==============================================");
    ensure_orig_loaded();
}

__attribute__((destructor))
static void on_unload() {
    LOGI("DisplayFeature interposer unloading");
    if (g_orig.dl) {
        dlclose(g_orig.dl);
        g_orig.dl = nullptr;
    }
}

} /* extern "C" */
