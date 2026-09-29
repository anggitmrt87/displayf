#pragma once

#include <dlfcn.h>
#include <string>

namespace dfhook {

/* ===================================================================
 * Resolve symbol dari handle dlopen.
 * Wrapper dengan error logging.
 * =================================================================== */

void* resolve_symbol(void* handle, const char* name);

/* Resolve beberapa kandidat nama, ambil yang pertama ketemu. */
void* resolve_first(void* handle, const char** names, int count);

/* Cek apakah symbol ada (tanpa log). */
bool has_symbol(void* handle, const char* name);

/* Cari symbol di semua library yang sudah di-load (via RTLD_DEFAULT). */
void* resolve_global(const char* name);

/* Cari symbol yang mungkin dimangle dengan nama alternatif. */
void* resolve_mangled(void* handle, const char* base_name);

} /* namespace dfhook */
