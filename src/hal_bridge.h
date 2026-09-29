#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "displayfeature/types.h"

namespace displayfeature {
namespace internal {

/* ===================================================================
 * Shell helper
 * =================================================================== */

struct ShellResult {
    int exit_code = -1;
    std::string out;   // stdout + stderr gabungan
    bool ok() const { return exit_code == 0; }
};

ShellResult run_cmd(const std::string& cmd);
ShellResult service_call(const std::string& service, int tx_code,
                         const std::vector<int32_t>& args = {});

/* ===================================================================
 * File helpers
 * =================================================================== */

bool file_exists(const std::string& p);
bool file_writable(const std::string& p);
std::string read_file(const std::string& p, bool* ok = nullptr);
bool write_file(const std::string& p, const std::string& data);

/* ===================================================================
 * String helpers
 * =================================================================== */

std::string trim(const std::string& s);
std::string to_lower(const std::string& s);
bool parse_service_int(const std::string& out, int* v);
bool parcel_perm_denied(const std::string& out);
bool parcel_arg_mismatch(const std::string& out);
std::string shell_escape(const std::string& s);

/* ===================================================================
 * HAL backend
 * =================================================================== */

class HalBackend {
public:
    virtual ~HalBackend() = default;

    virtual df_error_t open() = 0;
    virtual void close() = 0;
    virtual bool is_open() const = 0;

    virtual df_error_t call(int tx_code,
                            const std::vector<int32_t>& args,
                            int* out_result) = 0;

    virtual df_backend_t type() const = 0;
    virtual const char* name() const = 0;
};

/* Factory. Return nullptr jika backend tidak tersedia. */
std::unique_ptr<HalBackend> make_hidl_backend();
std::unique_ptr<HalBackend> make_service_backend();
std::unique_ptr<HalBackend> make_sysfs_backend();

/* Pilih backend terbaik yang tersedia. */
std::unique_ptr<HalBackend> make_auto_backend();

} /* namespace internal */
} /* namespace displayfeature */
