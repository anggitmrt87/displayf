#ifndef DISPLAYFEATURE_CLIENT_H
#define DISPLAYFEATURE_CLIENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "displayfeature/types.h"
#include "displayfeature/version.h"

namespace displayfeature {

/* ===================================================================
 * Error type
 * =================================================================== */

class Error {
public:
    Error() = default;
    Error(df_error_t code, std::string msg)
        : code_(code), msg_(std::move(msg)) {}

    df_error_t code() const noexcept { return code_; }
    const std::string& message() const noexcept { return msg_; }
    bool ok() const noexcept { return code_ == DF_OK; }
    explicit operator bool() const noexcept { return !ok(); }

    static Error success() { return {}; }

private:
    df_error_t code_ = DF_OK;
    std::string msg_;
};

/* ===================================================================
 * Result<T>
 * =================================================================== */

template <typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)), err_() {}
    Result(Error e) : err_(std::move(e)) {}

    bool ok() const noexcept { return err_.ok(); }
    const Error& error() const noexcept { return err_; }

    T& value() { return value_; }
    const T& value() const { return value_; }

    T value_or(T fallback) const {
        return ok() ? value_ : std::move(fallback);
    }

private:
    T value_{};
    Error err_;
};

template <>
class Result<void> {
public:
    Result() = default;
    Result(Error e) : err_(std::move(e)) {}
    bool ok() const noexcept { return err_.ok(); }
    const Error& error() const noexcept { return err_; }

private:
    Error err_;
};

/* ===================================================================
 * Configuration
 * =================================================================== */

struct Config {
    df_backend_t backend = DF_BACKEND_AUTO;
    bool quiet = false;
    int  timeout_ms = 5000;
};

/* ===================================================================
 * Client
 * =================================================================== */

class Client {
public:
    Client();
    explicit Client(const Config& cfg);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) noexcept;
    Client& operator=(Client&&) noexcept;

    /* ----- lifecycle ----- */
    Error open();
    Error open(const Config& cfg);
    void close();
    bool is_open() const noexcept;

    /* ----- info ----- */
    static bool is_supported_device(std::string* codename = nullptr);
    const std::string& device_codename() const noexcept;
    const std::string& panel_name() const noexcept;
    bool is_root() const noexcept;
    df_backend_t active_backend() const noexcept;

    /* ----- state ----- */
    Result<df_state_t> state() const;
    Result<std::string>  dumpsys() const;

    /* ----- features ----- */
    Error set_eyecare(int value);
    Error set_reading_mode(df_reading_mode_t mode);
    Error set_reading_mode_ct(int level);
    Error set_color_scheme(df_color_scheme_t scheme);
    Error set_color_scheme_ct(int level);
    Error set_hbm(bool on);
    Error set_cabc(int mode);

    /* ----- backlight ----- */
    Result<int>  get_backlight() const;
    Result<int>  get_backlight_max() const;
    Error        set_backlight(int value);

    /* ----- generic ----- */
    Result<int> call(int tx_code, const std::vector<int32_t>& args) const;

    /* ----- raw HIDL ----- */
    Result<int> hidl_set_feature(int display_id, int case_id,
                                 int mode_id, int cookie) const;

    /* ----- probing ----- */
    struct ProbeEntry {
        int code;
        bool arg_mismatch;
        bool perm_denied;
        bool ok;
        std::string raw;
    };
    Result<std::vector<ProbeEntry>> probe(int start = 1, int end = 40) const;

    /* ----- config ----- */
    void set_backend(df_backend_t b);
    void set_quiet(bool q);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/* ===================================================================
 * One-shot convenience helpers
 * =================================================================== */

namespace oneshot {

bool  is_supported_device(std::string* codename = nullptr);
Error toggle_reading_mode(bool on);
Error set_reading_mode(df_reading_mode_t m);
Error set_color_scheme(df_color_scheme_t s);
Error set_backlight(int value);
Result<df_state_t> state();

} /* namespace oneshot */

} /* namespace displayfeature */

#endif /* DISPLAYFEATURE_CLIENT_H */
