#ifndef HIDL_MIN_H
#define HIDL_MIN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal HIDL string ABI
 * Layout: { uint32_t size; uint8_t data[]; } untuk short string,
 * atau { uint32_t size; const char* data; } untuk long.
 * Untuk keamanan, kita definisikan sebagai opaque dan hanya
 * akses via API C.
 */
typedef struct hidl_string_abi {
    uint32_t size;
    uint32_t flags;   /* 0 = short, 1 = long */
    union {
        char     inline_data[20];
        const char* ptr;
    } data;
} hidl_string_abi;

/* HIDL return type adalah template, kita tidak bisa merepresentasikan
 * di C. Untuk keperluan interposer kita hanya butuh pointer opaque. */
typedef void* hidl_return_t;
typedef int32_t hidl_status_t;

/* Status codes */
#define HIDL_OK                  0
#define HIDL_ERR_UNKNOWN        -1
#define HIDL_ERR_UNSUPPORTED    -2
#define HIDL_ERR_BAD_VALUE      -3
#define HIDL_ERR_BAD_INDEX      -4

#ifdef __cplusplus
}
#endif

#endif /* HIDL_MIN_H */
