#include "displayfeature/client.h"
#include "hal_bridge.h"

#include <cstring>
#include <sstream>
#include <sys/system_properties.h>
#include <unistd.h>

namespace displayfeature {

using namespace displayfeature::internal;

/* ===================================================================
 * Impl
 * =================================================================== */

struct Client::Impl {
    Config cfg;
    std::unique_ptr<HalBackend> backend;
    std::string codename;
    std::string panel;
    bool root = false;
    bool open = false;

    bool detect_device();
    bool detect_panel();
};

bool Client::Impl::detect_device() {
    char v[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.device", v);
    codename.assign(v);          // FIXED: hindari pointer-bool warning
    return codename == DF_DEVICE_LIME   ||
           codename == DF_DEVICE_LEMON  ||
           codename == DF_DEVICE_POMELO ||
           codename == DF_DEVICE_CITRUS;
}

bool Client::Impl::detect_panel() {
    bool ok = false;
    std::string s = read_file(DF_PANEL_INFO_PATH, &ok);
    if (!ok) return false;
    // Format: "panel_name=dsi_nt36672d_xinli_v2_video_display"
    auto p = s.find("panel_name=");
    panel = (p == std::string::npos)
        ? trim(s)
        : trim(s.substr(p + 11));
    return true;
}

/* ===================================================================
 * Lifecycle
 * =================================================================== */

Client::Client() : impl_(std::make_unique<Impl>()) {}

Client::Client(const Config& cfg) : impl_(std::make_unique<Impl>()) {
    impl_->cfg = cfg;
}

Client::~Client() = default;
Client::Client(Client&&) noexcept = default;
Client& Client::operator=(Client&&) noexcept = default;

Error Client::open() { return open(impl_->cfg); }

Error Client::open(const Config& cfg) {
    impl_->cfg  = cfg;
    impl_->root = (getuid() == 0);
    impl_->detect_device();
    impl_->detect_panel();

    std::unique_ptr<HalBackend> b;
    switch (cfg.backend) {
        case DF_BACKEND_HIDL:    b = make_hidl_backend();    break;
        case DF_BACKEND_SERVICE: b = make_service_backend(); break;
        case DF_BACKEND_SYSFS:   b = make_sysfs_backend();   break;
        case DF_BACKEND_AUTO:
        default:                 b = make_auto_backend();    break;
    }
    if (!b) return {DF_ERR_SERVICE_NOT_FOUND, "no backend available"};

    df_error_t e = b->open();
    if (e != DF_OK) return {e, "backend open failed"};

    impl_->backend = std::move(b);
    impl_->open    = true;
    return Error::success();
}

void Client::close() {
    if (impl_->backend) impl_->backend->close();
    impl_->backend.reset();
    impl_->open = false;
}

bool Client::is_open() const noexcept {
    return impl_ && impl_->open;
}

/* ===================================================================
 * Info
 * =================================================================== */

bool Client::is_supported_device(std::string* codename) {
    char v[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.device", v);
    std::string c(v);            // FIXED: hindari pointer-bool warning
    if (codename) *codename = c;
    return c == DF_DEVICE_LIME   ||
           c == DF_DEVICE_LEMON  ||
           c == DF_DEVICE_POMELO ||
           c == DF_DEVICE_CITRUS;
}

const std::string& Client::device_codename() const noexcept {
    return impl_->codename;
}

const std::string& Client::panel_name() const noexcept {
    return impl_->panel;
}

bool Client::is_root() const noexcept {
    return impl_->root;
}

df_backend_t Client::active_backend() const noexcept {
    return impl_->backend ? impl_->backend->type() : DF_BACKEND_AUTO;
}

/* ===================================================================
 * State
 * =================================================================== */

Result<std::string> Client::dumpsys() const {
    ShellResult r = run_cmd("dumpsys " DF_SERVICE_NAME);
    if (!r.ok() || r.out.empty())
        return Error{DF_ERR_IO, "dumpsys failed"};
    return r.out;
}

static int parse_field(const std::string& d, const std::string& key) {
    std::string needle = "m" + key + "=";
    auto p = d.find(needle);
    if (p == std::string::npos) return 0;
    p += needle.size();
    auto e = d.find_first_of("\r\n", p);
    if (e == std::string::npos) e = d.size();
    try {
        return std::stoi(trim(d.substr(p, e - p)));
    } catch (...) {
        return 0;
    }
}

Result<df_state_t> Client::state() const {
    auto dump = dumpsys();
    if (!dump.ok()) return dump.error();

    df_state_t s{};
    const std::string& d = dump.value();

    s.reading_mode_enabled   = parse_field(d, "ReadingModeEnabled");
    s.reading_mode_type      = parse_field(d, "ReadingModeType");
    s.reading_mode_ct_level  = parse_field(d, "ReadingModeCTLevel");
    s.color_scheme_mode_type = parse_field(d, "ColorSchemeModeType");
    s.color_scheme_ct_level  = parse_field(d, "ColorSchemeCTLevel");
    s.game_hdr_enabled       = parse_field(d, "GameHdrEnabled");
    s.force_disable_eye_care = parse_field(d, "ForceDisableEyeCare");
    s.auto_adjust_enable     = parse_field(d, "AutoAdjustEnable");
    s.paper_color_type       = parse_field(d, "PaperColorType");

    auto cur = get_backlight();
    auto mx  = get_backlight_max();
    s.backlight_current = cur.ok() ? cur.value() : 0;
    s.backlight_max     = mx.ok()  ? mx.value()  : 4095;

    std::strncpy(s.codename, impl_->codename.c_str(),
                 sizeof(s.codename) - 1);
    std::strncpy(s.panel, impl_->panel.c_str(),
                 sizeof(s.panel) - 1);
    return s;
}

/* ===================================================================
 * Features
 * =================================================================== */

Error Client::set_eyecare(int value) {
    if (!impl_->open)
        return {DF_ERR_NOT_INITIALIZED, "client not opened"};
    int out = 0;
    df_error_t e = impl_->backend->call(TX_SET_EYECARE_SWITCH,
                                        {value}, &out);
    return e == DF_OK
        ? Error::success()
        : Error{e, "set_eyecare failed"};
}

Error Client::set_reading_mode(df_reading_mode_t m) {
    return set_eyecare(static_cast<int>(m));
}

Error Client::set_reading_mode_ct(int level) {
    if (!impl_->open)
        return {DF_ERR_NOT_INITIALIZED, "client not opened"};
    for (int tx = 11; tx <= 18; tx++) {
        int out = 0;
        if (impl_->backend->call(tx, {level}, &out) == DF_OK)
            return Error::success();
    }
    return {DF_ERR_UNSUPPORTED, "no tx code matched"};
}

Error Client::set_color_scheme(df_color_scheme_t s) {
    return set_reading_mode_ct(static_cast<int>(s));
}

Error Client::set_color_scheme_ct(int level) {
    return set_reading_mode_ct(level);
}

Error Client::set_hbm(bool on) {
    if (!impl_->open)
        return {DF_ERR_NOT_INITIALIZED, "client not opened"};
    for (int tx = 11; tx <= 18; tx++) {
        int out = 0;
        if (impl_->backend->call(tx, {on ? 1 : 0}, &out) == DF_OK)
            return Error::success();
    }
    return {DF_ERR_UNSUPPORTED, "no tx code matched"};
}

Error Client::set_cabc(int mode) {
    return set_reading_mode_ct(mode);
}

/* ===================================================================
 * Backlight
 * =================================================================== */

Result<int> Client::get_backlight() const {
    bool ok = false;
    std::string s = trim(read_file(BACKLIGHT_PATH, &ok));
    if (!ok)
        return Error{DF_ERR_IO, "backlight not readable"};
    try {
        return std::stoi(s);
    } catch (...) {
        return Error{DF_ERR_IO, "invalid backlight value"};
    }
}

Result<int> Client::get_backlight_max() const {
    bool ok = false;
    std::string s = trim(read_file(MAX_BRIGHTNESS_PATH, &ok));
    if (!ok)
        return Error{DF_ERR_IO, "max_brightness not readable"};
    try {
        return std::stoi(s);
    } catch (...) {
        return Error{DF_ERR_IO, "invalid max_brightness value"};
    }
}

Error Client::set_backlight(int value) {
    auto mx = get_backlight_max();
    int m = mx.ok() ? mx.value() : 4095;
    if (value < 0) value = 0;
    if (value > m) value = m;
    if (!write_file(BACKLIGHT_PATH, std::to_string(value)))
        return {DF_ERR_IO, "backlight write failed"};
    return Error::success();
}

/* ===================================================================
 * Generic
 * =================================================================== */

Result<int> Client::call(int tx_code,
                         const std::vector<int32_t>& args) const {
    if (!impl_->open)
        return Error{DF_ERR_NOT_INITIALIZED, "client not opened"};
    int out = 0;
    df_error_t e = impl_->backend->call(tx_code, args, &out);
    if (e != DF_OK)
        return Error{e, "call failed"};
    return out;
}

Result<int> Client::hidl_set_feature(int did, int cid,
                                     int mid, int ck) const {
    return call(1, {did, cid, mid, ck});
}

/* ===================================================================
 * Probe
 * =================================================================== */

Result<std::vector<Client::ProbeEntry>>
Client::probe(int start, int end) const {
    if (!impl_->open)
        return Error{DF_ERR_NOT_INITIALIZED, "client not opened"};

    std::vector<ProbeEntry> v;
    for (int code = start; code <= end; code++) {
        ProbeEntry e{};
        e.code = code;

        auto r0 = service_call(DF_SERVICE_NAME, code);
        auto r1 = service_call(DF_SERVICE_NAME, code, {1});

        e.perm_denied  = parcel_perm_denied(r0.out) ||
                         parcel_perm_denied(r1.out);
        e.arg_mismatch = parcel_arg_mismatch(r0.out) &&
                         parcel_arg_mismatch(r1.out);
        e.ok           = r0.ok() && !r0.out.empty();
        e.raw          = trim(r0.out).substr(0, 200);

        v.push_back(std::move(e));
    }
    return v;
}

void Client::set_backend(df_backend_t b) { impl_->cfg.backend = b; }
void Client::set_quiet(bool q)           { impl_->cfg.quiet = q; }

/* ===================================================================
 * oneshot
 * =================================================================== */

namespace oneshot {

bool is_supported_device(std::string* c) {
    return Client::is_supported_device(c);
}

static Client make_client() {
    Client c;
    c.open();
    return c;
}

Error toggle_reading_mode(bool on) {
    auto c = make_client();
    return c.set_reading_mode(on ? DF_READING_PAPER : DF_READING_OFF);
}

Error set_reading_mode(df_reading_mode_t m) {
    auto c = make_client();
    return c.set_reading_mode(m);
}

Error set_color_scheme(df_color_scheme_t s) {
    auto c = make_client();
    return c.set_color_scheme(s);
}

Error set_backlight(int v) {
    auto c = make_client();
    return c.set_backlight(v);
}

Result<df_state_t> state() {
    auto c = make_client();
    return c.state();
}

} /* namespace oneshot */

} /* namespace displayfeature */
