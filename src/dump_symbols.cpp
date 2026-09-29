/*
 * dump_symbols.cpp
 * Utility untuk dump symbol dari .so.
 * Bisa jalan di device (aarch64/arm) atau host (x86_64).
 *
 * Usage: dump_symbols <path/to/.so> [symbol1 symbol2 ...]
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <android/log.h>

#define LOG_TAG "df-dump"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/* Symbol umum yang kita cari di displayfeature.default.so */
static const char* DEFAULT_SYMBOLS[] = {
    "HMI",
    "HAL_MODULE_INFO_SYM",

    /* HIDL entry */
    "HIDL_FETCH_IDisplayFeature",
    "_ZN6vendor6xiaomi8hardware14displayfeature4V1_014implementation14DisplayFeatureC1EPK11hw_module_t",
    "_ZN6vendor6xiaomi8hardware14displayfeature4V1_014implementation14DisplayFeatureC2EPK11hw_module_t",

    /* DisplayFeatureHal methods */
    "_ZN7android17DisplayFeatureHal4openEPK11hw_module_tPKcPP11hw_device_t",
    "_ZN7android17DisplayFeatureHal5closeEP11hw_device_t",
    "_ZN7android17DisplayFeatureHal4initEv",
    "_ZN7android17DisplayFeatureHal6deinitEv",
    "_ZN7android17DisplayFeatureHal11getFunctionEP9df_devicei",
    "_ZN7android17DisplayFeatureHal16setFeatureEnableEP9df_deviceiiii",
    "_ZN7android17DisplayFeatureHal17setFunctionEnableEP9df_deviceiiii",
    "_ZN7android17DisplayFeatureHal15getCapabilitiesEP9df_devicePjPi",
    "_ZN7android17DisplayFeatureHal11sendMessageEP9df_deviceiiRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE",
    "_ZN7android17DisplayFeatureHal9log_levelE",
    "_ZN7android17DisplayFeatureHal5mLockE",

    /* DisplayEffect */
    "_ZN7android13DisplayEffect9HandleHBMEii",
    "_ZN7android13DisplayEffect13HandleEyeCareEii",
    "_ZN7android13DisplayEffect20HandleCabcModeCustomEii",
    "_ZN7android13DisplayEffect24HandleKeepWhitePointSRGBEii",
    "_ZN7android13DisplayEffect15HandleNightModeEii",

    nullptr
};

static int on_phdr(struct dl_phdr_info* info, size_t /*size*/, void* /*data*/) {
    if (info->dlpi_name && info->dlpi_name[0]) {
        fprintf(stderr, "  [loaded] %s @ %p\n", info->dlpi_name,
                (void*)info->dlpi_addr);
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path/to/.so> [symbol...]\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];
    fprintf(stderr, "=== Loading: %s ===\n", path);

    void* h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 2;
    }

    fprintf(stderr, "=== dl_iterate_phdr ===\n");
    dl_iterate_phdr(on_phdr, nullptr);

    fprintf(stderr, "=== Symbols ===\n");
    if (argc > 2) {
        /* Custom symbol list */
        for (int i = 2; i < argc; i++) {
            dlerror();
            void* p = dlsym(h, argv[i]);
            const char* err = dlerror();
            if (err) {
                printf("  %-70s = (not found)\n", argv[i]);
            } else {
                printf("  %-70s = %p\n", argv[i], p);
            }
        }
    } else {
        /* Default list */
        for (int i = 0; DEFAULT_SYMBOLS[i]; i++) {
            dlerror();
            void* p = dlsym(h, DEFAULT_SYMBOLS[i]);
            const char* err = dlerror();
            if (!err) {
                printf("  %-70s = %p\n", DEFAULT_SYMBOLS[i], p);
            }
        }
    }

    dlclose(h);
    return 0;
}
