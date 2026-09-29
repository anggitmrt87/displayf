#pragma once

#include "hw_module.h"

namespace dfhook {

/* ===================================================================
 * Function pointer types
 * =================================================================== */

typedef int (*orig_open_fn)(const hw_module_t*, const char*, hw_device_t**);
typedef int (*orig_close_fn)(hw_device_t*);

/* ===================================================================
 * Set original open/close (dipanggil oleh interposer setelah dlopen)
 * =================================================================== */

void set_original_open(orig_open_fn fn);
void set_original_close(orig_close_fn fn);

/* ===================================================================
 * Install hook pada device yang baru dibuka.
 * =================================================================== */

void install_device_hook(hw_device_t* device);

/* ===================================================================
 * Hook function yang dipasang ke module->methods->open
 * =================================================================== */

int hooked_open(const hw_module_t* module, const char* id,
                hw_device_t** device);

int hooked_close(hw_device_t* device);

} /* namespace dfhook */
