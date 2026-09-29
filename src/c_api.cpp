#include "displayfeature/c_api.h"
#include "displayfeature/client.h"

#include <cstdlib>
#include <cstring>

using displayfeature::Client;
using displayfeature::Config;
using displayfeature::Error;

/* ===================================================================
 * Lifecycle
 * =================================================================== */

extern "C" {

df_client_t* df_client_create(void) {
    return reinterpret_cast<df_client_t*>(new (std::nothrow) Client());
}

df_client_t* df_client_create_with_backend(df_backend_t b) {
    Config c; c.backend = b;
    return reinterpret_cast<df_client_t*>(new (std::nothrow) Client(c));
}

df_error_t df_client_open(df_client_t* c) {
    if (!c) return DF_ERR_INVALID_ARG;
    auto cli = reinterpret_cast<Client*>(c);
    Error e = cli->open();
    return e.code();
}

void df_client_destroy(df_client_t* c) {
    delete reinterpret_cast<Client*>(c);
}

/* ===================================================================
 * Info
 * =================================================================== */

int df_is_supported_device(char* out, size_t len) {
    std::string code;
    bool ok = Client::is_supported_device(&code);
    if (out && len) {
        std::strncpy(out, code.c_str(), len - 1);
        out[len - 1] = 0;
    }
    return ok ? 1 : 0;
}

df_error_t df_device_codename(df_client_t* c, char* buf, size_t len) {
    if (!c || !buf || !len) return DF_ERR_INVALID_ARG;
    auto cli = reinterpret_cast<Client*>(c);
    std::strncpy(buf, cli->device_codename().c_str(), len - 1);
    buf[len - 1] = 0;
    return DF_OK;
}

df_error_t df_panel_name(df_client_t* c, char* buf, size_t len) {
    if (!c || !buf || !len) return DF_ERR_INVALID_ARG;
    auto cli = reinterpret_cast<Client*>(c);
    std::strncpy(buf, cli->panel_name().c_str(), len - 1);
    buf[len - 1] = 0;
    return DF_OK;
}

int df_is_root(void) {
    extern int getuid(void);
    return getuid() == 0 ? 1 : 0;
}

df_backend_t df_active_backend(df_client_t* c) {
    if (!c) return DF_BACKEND_AUTO;
    return reinterpret_cast<Client*>(c)->active_backend();
}

/* ===================================================================
 * State
 * =================================================================== */

df_error_t df_get_state(df_client_t* c, df_state_t* out) {
    if (!c || !out) return DF_ERR_INVALID_ARG;
    auto cli = reinterpret_cast<Client*>(c);
    auto r = cli->state();
    if (!r.ok()) return r.error().code();
    *out = r.value();
    return DF_OK;
}

df_error_t df_dumpsys(df_client_t* c, char** out_str) {
    if (!c || !out_str) return DF_ERR_INVALID_ARG;
    auto cli = reinterpret_cast<Client*>(c);
    auto r = cli->dumpsys();
    if (!r.ok()) return r.error().code();
    *out_str = ::strdup(r.value().c_str());
    return *out_str ? DF_OK : DF_ERR_IO;
}

void df_free_string(char* s) { std::free(s); }

/* ===================================================================
 * Features
 * =================================================================== */

df_error_t df_set_eyecare(df_client_t* c, int value) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_eyecare(value).code();
}
df_error_t df_set_reading_mode(df_client_t* c, df_reading_mode_t m) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_reading_mode(m).code();
}
df_error_t df_set_reading_mode_ct(df_client_t* c, int level) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_reading_mode_ct(level).code();
}
df_error_t df_set_color_scheme(df_client_t* c, df_color_scheme_t s) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_color_scheme(s).code();
}
df_error_t df_set_color_scheme_ct(df_client_t* c, int level) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_color_scheme_ct(level).code();
}
df_error_t df_set_hbm(df_client_t* c, int on) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_hbm(on != 0).code();
}
df_error_t df_set_cabc(df_client_t* c, int mode) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_cabc(mode).code();
}

/* ===================================================================
 * Backlight
 * =================================================================== */

df_error_t df_get_backlight(df_client_t* c, int* out) {
    if (!c || !out) return DF_ERR_INVALID_ARG;
    auto r = reinterpret_cast<Client*>(c)->get_backlight();
    if (!r.ok()) return r.error().code();
    *out = r.value();
    return DF_OK;
}
df_error_t df_get_backlight_max(df_client_t* c, int* out) {
    if (!c || !out) return DF_ERR_INVALID_ARG;
    auto r = reinterpret_cast<Client*>(c)->get_backlight_max();
    if (!r.ok()) return r.error().code();
    *out = r.value();
    return DF_OK;
}
df_error_t df_set_backlight(df_client_t* c, int v) {
    if (!c) return DF_ERR_INVALID_ARG;
    return reinterpret_cast<Client*>(c)->set_backlight(v).code();
}

/* ===================================================================
 * Generic
 * =================================================================== */

df_error_t df_call(df_client_t* c, int tx_code,
                   const int32_t* args, size_t nargs, int* out) {
    if (!c) return DF_ERR_INVALID_ARG;
    std::vector<int32_t> v(args, args + nargs);
    auto r = reinterpret_cast<Client*>(c)->call(tx_code, v);
    if (!r.ok()) return r.error().code();
    if (out) *out = r.value();
    return DF_OK;
}

df_error_t df_hidl_set_feature(df_client_t* c,
                               int did, int cid, int mid, int ck,
                               int* out) {
    if (!c) return DF_ERR_INVALID_ARG;
    auto r = reinterpret_cast<Client*>(c)->hidl_set_feature(did, cid, mid, ck);
    if (!r.ok()) return r.error().code();
    if (out) *out = r.value();
    return DF_OK;
}

/* ===================================================================
 * Probing
 * =================================================================== */

df_error_t df_probe(df_client_t* c, int start, int end,
                    df_probe_entry_t** out_entries, size_t* out_count) {
    if (!c || !out_entries || !out_count) return DF_ERR_INVALID_ARG;
    auto r = reinterpret_cast<Client*>(c)->probe(start, end);
    if (!r.ok()) return r.error().code();

    auto& vec = r.value();
    auto* arr = (df_probe_entry_t*)std::calloc(vec.size(), sizeof(df_probe_entry_t));
    if (!arr) return DF_ERR_IO;

    for (size_t i = 0; i < vec.size(); i++) {
        arr[i].code         = vec[i].code;
        arr[i].arg_mismatch = vec[i].arg_mismatch ? 1 : 0;
        arr[i].perm_denied  = vec[i].perm_denied ? 1 : 0;
        arr[i].ok           = vec[i].ok ? 1 : 0;
        arr[i].raw          = ::strdup(vec[i].raw.c_str());
    }
    *out_entries = arr;
    *out_count   = vec.size();
    return DF_OK;
}

void df_probe_free(df_probe_entry_t* e, size_t n) {
    if (!e) return;
    for (size_t i = 0; i < n; i++) std::free((void*)e[i].raw);
    std::free(e);
}

/* ===================================================================
 * Error string
 * =================================================================== */

const char* df_error_string(df_error_t e) {
    switch (e) {
        case DF_OK:                    return "OK";
        case DF_ERR_INVALID_ARG:       return "invalid argument";
        case DF_ERR_NOT_INITIALIZED:   return "not initialized";
        case DF_ERR_SERVICE_NOT_FOUND: return "service not found";
        case DF_ERR_PERMISSION_DENIED: return "permission denied";
        case DF_ERR_TRANSACTION_FAIL:  return "transaction failed";
        case DF_ERR_IO:                return "I/O error";
        case DF_ERR_UNSUPPORTED:       return "unsupported";
        case DF_ERR_TIMEOUT:           return "timeout";
        default:                       return "unknown error";
    }
}

/* ===================================================================
 * oneshot
 * =================================================================== */

df_error_t df_oneshot_set_reading_mode(df_reading_mode_t m) {
    auto c = Client();
    c.open();
    return c.set_reading_mode(m).code();
}

df_error_t df_oneshot_set_backlight(int v) {
    auto c = Client();
    c.open();
    return c.set_backlight(v).code();
}

df_error_t df_oneshot_state(df_state_t* out) {
    if (!out) return DF_ERR_INVALID_ARG;
    auto c = Client();
    c.open();
    auto r = c.state();
    if (!r.ok()) return r.error().code();
    *out = r.value();
    return DF_OK;
}

} /* extern "C" */
