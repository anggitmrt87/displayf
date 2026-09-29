#ifndef HW_MODULE_MIN_H
#define HW_MODULE_MIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal struct definitions sesuai AOSP hardware/libhardware/include/hardware/hardware.h */

struct hw_module_t;
struct hw_module_methods_t;
struct hw_device_t;

typedef struct hw_module_t {
    uint32_t tag;
    uint16_t module_api_version;
    uint16_t hal_api_version;
    const char* id;
    const char* name;
    const char* author;
    struct hw_module_methods_t* methods;
    void* dso;
    uint32_t reserved[32 - 7];
} hw_module_t;

typedef struct hw_module_methods_t {
    int (*open)(const struct hw_module_t* module, const char* id,
                struct hw_device_t** device);
} hw_module_methods_t;

typedef struct hw_device_t {
    uint32_t tag;
    uint32_t version;
    struct hw_module_t* module;
    uint32_t reserved[12];
    int (*close)(struct hw_device_t* device);
} hw_device_t;

#define HAL_MODULE_INFO_SYM         HMI
#define HAL_MODULE_INFO_SYM_AS_STR  "HMI"

#define MAKE_TAG_CONSTANT(A, B, C, D) \
    (((A) << 24) | ((B) << 16) | ((C) << 8) | (D))

#define HARDWARE_MODULE_TAG \
    MAKE_TAG_CONSTANT('H', 'W', 'M', 'T')
#define HARDWARE_DEVICE_TAG \
    MAKE_TAG_CONSTANT('H', 'W', 'D', 'T')

#define HARDWARE_MAKE_API_VERSION(maj, min) \
    ((((maj) & 0xff) << 8) | ((min) & 0xff))
#define HARDWARE_MAKE_API_VERSION_2(maj, min, hmi) \
    ((((maj) & 0xff) << 16) | (((min) & 0xff) << 8) | ((hmi) & 0xff))

#ifdef __cplusplus
}
#endif

#endif /* HW_MODULE_MIN_H */
