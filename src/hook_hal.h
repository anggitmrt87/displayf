#pragma once

#include "hw_module.h"

namespace dfhook {

/* ===================================================================
 * Install hook pada device yang baru dibuka.
 * Dipanggil dari hooked_open() setelah original open() berhasil.
 *
 * Saat ini hanya log. Akan di-update setelah layout df_device
 * dikonfirmasi dari symbol analysis.
 * =================================================================== */

void install_device_hook(hw_device_t* device);

/* Hook function yang bisa dipasang ke module->methods->open */
int hooked_open(const hw_module_t* module, const char* id,
                hw_device_t** device);

/* Hook untuk close */
int hooked_close(hw_device_t* device);

} /* namespace dfhook */
