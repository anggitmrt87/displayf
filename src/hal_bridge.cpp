#include "hal_bridge.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <dlfcn.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#ifdef DF_HAVE_ANDROID_LOG
#include <android/log.h>
#define DFLOG(...) __android_log_print(ANDROID_LOG_INFO, "libdf", __VA_ARGS__)
#else
#define DFLOG(...) do {} while (0)
#endif

namespace displayfeature {
namespace internal {

/* ===================================================================
 * Shell
 * =================================================================== */

std::string shell_escape(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
}

ShellResult run_cmd(const std::string& cmd) {
    ShellResult r;
    std::string full = cmd + " 2>&1";
    FILE* fp = popen(full.c_str(), "r");
    if (!fp) { r.exit_code = -1; return r; }

    std::array<char, 4096> buf{};
    size_t n;
    while ((n = fread(buf.data(), 1, buf.size(), fp)) > 0) {
        r.out.append(buf.data(), n);
    }
    int st = pclose(fp);
    r.exit_code = (st != -1 && WIFEXITED(st)) ? WEXITSTATUS(st) : -1;
    return r;
}

ShellResult service_call(const std::string& service, int code,
                         const std::vector<int32_t>& args) {
    std::string cmd = "service call ";
    cmd += shell_escape(service);
    cmd += ' ';
    cmd += std::to_string(code);
    for (int a : args) {
        cmd += " i32 ";
        cmd += std::to_string(a);
    }
    return run_cmd(cmd);
}

/* ===================================================================
 * Files
 * =================================================================== */

bool file_exists(const std::string& p) { return access(p.c_str(), F_OK) == 0; }
bool file_writable(const std::string& p) { return access(p.c_str(), W_OK) == 0; }

std::string read_file(const std::string& p, bool* ok) {
    std::ifstream f(p);
    if (!f.is_open()) { if (ok) *ok = false; return {}; }
    std::string s; std::getline(f, s);
    if (ok) *ok = true;
    return s;
}

bool write_file(const std::string& p, const std::string& d) {
    std::ofstream f(p);
    if (!f.is_open()) return false;
    f << d;
    return f.good();
}

/* ===================================================================
 * Strings
 * =================================================================== */

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) b++;
    while (e > b && std::isspace((unsigned char)s[e - 1])) e--;
    return s.substr(b, e - b);
}

std::string to_lower(const std::string& s) {
    std::string r = s;
    for (auto& c : r) c = (char)std::tolower((unsigned char)c);
    return r;
}

bool parse_service_int(const std::string& out, int* v) {
    auto p = out.find("Parcel(");
    if (p == std::string::npos) return false;
    p += 7;
    while (p < out.size() && (out[p] == ' ' || out[p] == '\t')) p++;
    if (p + 8 > out.size()) return false;
    char hex[9] = {0};
    for (int i = 0; i < 8; i++) {
        char c = out[p + i];
        if (!std::isxdigit((unsigned char)c)) return false;
        hex[i] = c;
    }
    *v = (int)strtoul(hex, nullptr, 16);
    return true;
}

bool parcel_perm_denied(const std::string& out) {
    return out.find("Permission") != std::string::npos ||
           out.find("denied") != std::string::npos;
}

bool parcel_arg_mismatch(const std::string& out) {
    return out.find("not fully consumed") != std::string::npos;
}

/* ===================================================================
 * Service backend (paling portable)
 * =================================================================== */

class ServiceBackend : public HalBackend {
public:
    static constexpr const char* kService = "displayfeature";

    df_error_t open() override {
        // Cek service ada
        ShellResult r = run_cmd("service list | grep -w displayfeature");
        if (r.out.find("displayfeature") == std::string::npos)
            return DF_ERR_SERVICE_NOT_FOUND;
        open_ = true;
        return DF_OK;
    }

    void close() override { open_ = false; }
    bool is_open() const override { return open_; }

    df_error_t call(int tx, const std::vector<int32_t>& args,
                    int* out) override {
        if (!open_) return DF_ERR_NOT_INITIALIZED;
        auto r = service_call(kService, tx, args);
        if (parcel_perm_denied(r.out)) return DF_ERR_PERMISSION_DENIED;
        if (parcel_arg_mismatch(r.out)) return DF_ERR_INVALID_ARG;
        if (!r.ok()) return DF_ERR_TRANSACTION_FAIL;
        if (out) parse_service_int(r.out, out);
        return DF_OK;
    }

    df_backend_t type() const override { return DF_BACKEND_SERVICE; }
    const char* name() const override { return "service"; }

private:
    bool open_ = false;
};

/* ===================================================================
 * HIDL backend (via dlopen ke libbinder)
 * =================================================================== */

class HidlBackend : public HalBackend {
public:
    df_error_t open() override {
        if (open_) return DF_OK;

        void* libbinder = dlopen("libbinder.so", RTLD_NOW);
        void* libutils  = dlopen("libutils.so",  RTLD_NOW);
        if (!libbinder || !libutils) {
            DFLOG("dlopen libbinder/utils gagal");
            return DF_ERR_SERVICE_NOT_FOUND;
        }

        /* Symbol lookups ---------------------------------------------- */

        auto sym = [](void* h, const char* name) { return dlsym(h, name); };

        // defaultServiceManager()
        fn_default_sm_ = (fn_default_sm_t)sym(libbinder,
            "_ZN7android21defaultServiceManagerEv");

        // IServiceManager::getService(String16 const&)
        fn_sm_get_ = (fn_sm_get_t)sym(libbinder,
            "_ZN7android14IServiceManager10getServiceERKNS_8String16E");

        // IBinder::transact(uint32_t, Parcel const&, Parcel*, uint32_t)
        fn_transact_ = (fn_transact_t)sym(libbinder,
            "_ZN7android7IBinder8transactEjRKNS_6ParcelEPS1_j");

        // Parcel::obtain()
        fn_parcel_obtain_ = (fn_parcel_obtain_t)sym(libbinder,
            "_ZN7android6Parcel6obtainEv");

        // Parcel::~Parcel()
        fn_parcel_dtor_ = (fn_parcel_dtor_t)sym(libbinder,
            "_ZN7android6ParcelD1Ev");

        // Parcel::writeInt32(int32_t)
        fn_parcel_wi32_ = (fn_parcel_wi32_t)sym(libbinder,
            "_ZN7android6Parcel10writeInt32Ei");

        // Parcel::readInt32()
        fn_parcel_ri32_ = (fn_parcel_ri32_t)sym(libbinder,
            "_ZN7android6Parcel9readInt32Ev");

        // String16::String16(char const*)
        fn_string16_ctor_ = (fn_string16_ctor_t)sym(libutils,
            "_ZN7android8String16C1EPKc");

        // String16::~String16()
        fn_string16_dtor_ = (fn_string16_dtor_t)sym(libutils,
            "_ZN7android8String16D1Ev");

        if (!fn_default_sm_ || !fn_sm_get_ || !fn_transact_ ||
            !fn_parcel_obtain_ || !fn_parcel_wi32_ || !fn_parcel_ri32_) {
            DFLOG("satu atau lebih symbol tidak ada");
            dlclose(libbinder);
            dlclose(libutils);
            return DF_ERR_SERVICE_NOT_FOUND;
        }

        /* Ambil service --------------------------------------------- */
        void* sm = fn_default_sm_();
        if (!sm) return DF_ERR_SERVICE_NOT_FOUND;

        // Siapkan String16 di stack (ukuran 24 byte cukup untuk name pendek)
        char name_buf[64] = {0};
        const char* svc_name =
            "vendor.xiaomi.hardware.displayfeature@1.0::IDisplayFeature";
        if (fn_string16_ctor_) fn_string16_ctor_(name_buf, svc_name);

        binder_ = fn_sm_get_(sm, name_buf);
        if (fn_string16_dtor_) fn_string16_dtor_(name_buf);

        if (!binder_) {
            DFLOG("HIDL service tidak ditemukan");
            return DF_ERR_SERVICE_NOT_FOUND;
        }

        libbinder_ = libbinder;
        libutils_  = libutils;
        open_ = true;
        DFLOG("HIDL backend siap");
        return DF_OK;
    }

    void close() override {
        binder_ = nullptr;
        if (libbinder_) dlclose(libbinder_);
        if (libutils_)  dlclose(libutils_);
        libbinder_ = libutils_ = nullptr;
        open_ = false;
    }

    bool is_open() const override { return open_; }

    df_error_t call(int tx, const std::vector<int32_t>& args,
                    int* out) override {
        if (!open_) return DF_ERR_NOT_INITIALIZED;

        void* data  = fn_parcel_obtain_();
        void* reply = fn_parcel_obtain_();
        if (!data || !reply) return DF_ERR_TRANSACTION_FAIL;

        // Kita tidak menulis interface token (mengandalkan HIDL yang
        // membaca dari descriptor); untuk kompatibilitas lebih baik
        // sertakan saat call pertama.
        for (int a : args) fn_parcel_wi32_(data, a);

        int err = fn_transact_(binder_, (uint32_t)tx, data, reply, 0);
        int result = 0;
        if (out) fn_parcel_ri32_(reply, &result);

        fn_parcel_dtor_(data);
        fn_parcel_dtor_(reply);

        if (err != 0) return DF_ERR_TRANSACTION_FAIL;
        if (out) *out = result;
        return DF_OK;
    }

    df_backend_t type() const override { return DF_BACKEND_HIDL; }
    const char* name() const override { return "hidl"; }

private:
    using fn_default_sm_t = void* (*)();
    using fn_sm_get_t     = void* (*)(void*, void*);
    using fn_transact_t   = int   (*)(void*, uint32_t, void*, void*, uint32_t);
    using fn_parcel_obtain_t = void* (*)();
    using fn_parcel_dtor_t   = void  (*)(void*);
    using fn_parcel_wi32_t   = int   (*)(void*, int32_t);
    using fn_parcel_ri32_t   = int   (*)(void*, int32_t*);
    using fn_string16_ctor_t = void  (*)(void*, const char*);
    using fn_string16_dtor_t = void  (*)(void*);

    void* libbinder_ = nullptr;
    void* libutils_  = nullptr;
    void* binder_    = nullptr;

    fn_default_sm_t      fn_default_sm_      = nullptr;
    fn_sm_get_t          fn_sm_get_          = nullptr;
    fn_transact_t        fn_transact_        = nullptr;
    fn_parcel_obtain_t   fn_parcel_obtain_   = nullptr;
    fn_parcel_dtor_t     fn_parcel_dtor_     = nullptr;
    fn_parcel_wi32_t     fn_parcel_wi32_     = nullptr;
    fn_parcel_ri32_t     fn_parcel_ri32_     = nullptr;
    fn_string16_ctor_t   fn_string16_ctor_   = nullptr;
    fn_string16_dtor_t   fn_string16_dtor_   = nullptr;

    bool open_ = false;
};

/* ===================================================================
 * Sysfs backend (hanya backlight)
 * =================================================================== */

class SysfsBackend : public HalBackend {
public:
    static constexpr const char* kBrightness =
        "/sys/class/backlight/panel0-backlight/brightness";
    static constexpr const char* kMax =
        "/sys/class/backlight/panel0-backlight/max_brightness";

    df_error_t open() override {
        open_ = file_exists(kBrightness);
        return open_ ? DF_OK : DF_ERR_SERVICE_NOT_FOUND;
    }
    void close() override { open_ = false; }
    bool is_open() const override { return open_; }

    df_error_t call(int, const std::vector<int32_t>&, int*) override {
        // Tidak mendukung transaksi generik
        return DF_ERR_UNSUPPORTED;
    }

    df_backend_t type() const override { return DF_BACKEND_SYSFS; }
    const char* name() const override { return "sysfs"; }

private:
    bool open_ = false;
};

/* ===================================================================
 * Factory
 * =================================================================== */

std::unique_ptr<HalBackend> make_hidl_backend() {
    return std::make_unique<HidlBackend>();
}
std::unique_ptr<HalBackend> make_service_backend() {
    return std::make_unique<ServiceBackend>();
}
std::unique_ptr<HalBackend> make_sysfs_backend() {
    return std::make_unique<SysfsBackend>();
}

std::unique_ptr<HalBackend> make_auto_backend() {
    // Coba HIDL dulu (paling cepat), lalu service, terakhir sysfs.
    {
        auto b = make_hidl_backend();
        if (b->open() == DF_OK) return b;
    }
    {
        auto b = make_service_backend();
        if (b->open() == DF_OK) return b;
    }
    {
        auto b = make_sysfs_backend();
        if (b->open() == DF_OK) return b;
    }
    return nullptr;
}

} /* namespace internal */
} /* namespace displayfeature */
