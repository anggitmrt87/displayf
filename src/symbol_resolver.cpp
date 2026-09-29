#include "symbol_resolver.h"

#include <android/log.h>
#include <cstring>
#include <vector>

#define LOG_TAG "df-hook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace dfhook {

void* resolve_symbol(void* handle, const char* name) {
    if (!handle || !name) return nullptr;

    /* Clear error */
    dlerror();

    void* sym = dlsym(handle, name);
    const char* err = dlerror();
    if (err) {
        LOGW("dlsym(%s): %s", name, err);
        return nullptr;
    }
    return sym;
}

void* resolve_first(void* handle, const char** names, int count) {
    for (int i = 0; i < count; i++) {
        if (!names[i]) continue;
        dlerror();
        void* sym = dlsym(handle, names[i]);
        if (!dlerror() && sym) {
            LOGI("resolved: %s = %p", names[i], sym);
            return sym;
        }
    }
    return nullptr;
}

bool has_symbol(void* handle, const char* name) {
    if (!handle || !name) return false;
    dlerror();
    dlsym(handle, name);
    return dlerror() == nullptr;
}

void* resolve_global(const char* name) {
    if (!name) return nullptr;
    dlerror();
    void* sym = dlsym(RTLD_DEFAULT, name);
    if (dlerror()) return nullptr;
    return sym;
}

void* resolve_mangled(void* handle, const char* base_name) {
    if (!handle || !base_name) return nullptr;

    /* Coba variasi umum: C++ mangling untuk fungsi umum
     * Base name sering berupa seperti "setFeatureEnable" — kita tidak
     * bisa generate mangling tanpa tahu signature lengkap, tapi kita
     * bisa coba beberapa pola. */

    std::vector<std::string> candidates;
    candidates.push_back(base_name);

    /* C-style */
    candidates.push_back(std::string("_") + base_name);

    for (const auto& c : candidates) {
        if (has_symbol(handle, c.c_str())) {
            return resolve_symbol(handle, c.c_str());
        }
    }
    return nullptr;
}

} /* namespace dfhook */
