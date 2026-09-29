#include "displayfeature/c_api.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    char code[32] = {0};
    if (!df_is_supported_device(code, sizeof(code))) {
        fprintf(stderr, "device tidak didukung: %s\n", code);
        return 1;
    }
    printf("device: %s\n", code);

    df_client_t* c = df_client_create();
    if (!c) return 2;

    if (df_client_open(c) != DF_OK) {
        fprintf(stderr, "open failed\n");
        df_client_destroy(c);
        return 3;
    }

    df_state_t s;
    if (df_get_state(c, &s) == DF_OK) {
        printf("reading_enabled = %d\n", s.reading_mode_enabled);
        printf("backlight       = %d / %d\n",
               s.backlight_current, s.backlight_max);
    }

    /* Coba nyalakan reading mode */
    df_error_t e = df_set_reading_mode(c, DF_READING_PAPER);
    printf("set_reading_mode = %d (%s)\n", e, df_error_string(e));

    /* Baca ulang */
    if (df_get_state(c, &s) == DF_OK) {
        printf("after: reading_enabled = %d\n", s.reading_mode_enabled);
    }

    df_client_destroy(c);
    return 0;
}
