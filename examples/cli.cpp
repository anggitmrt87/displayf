#include "displayfeature/client.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace displayfeature;

static void usage() {
    std::printf(
        "dfc-cli — demo pemakaian libdisplayfeature\n"
        "\n"
        "Usage: dfc-cli <command>\n"
        "  info                 Info device & backend\n"
        "  state                Baca state\n"
        "  readmode on|off      Toggle reading mode\n"
        "  readmode paper|soft\n"
        "  backlight [N]        Baca/set backlight\n"
        "  probe [start] [end]  Brute-force tx code\n"
    );
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(); return 1; }
    std::string cmd = argv[1];

    Client c;
    auto e = c.open();
    if (e) {
        std::fprintf(stderr, "open failed: %s\n", e.message().c_str());
        return 2;
    }

    if (cmd == "info") {
        std::printf("codename : %s\n", c.device_codename().c_str());
        std::printf("panel    : %s\n", c.panel_name().c_str());
        std::printf("root     : %s\n", c.is_root() ? "yes" : "no");
        std::printf("backend  : %d\n", (int)c.active_backend());
        std::printf("supported: %s\n",
                    Client::is_supported_device(nullptr) ? "yes" : "no");
        return 0;
    }

    if (cmd == "state") {
        auto r = c.state();
        if (!r.ok()) { std::fprintf(stderr, "state failed\n"); return 2; }
        auto& s = r.value();
        std::printf("reading_enabled : %d\n", s.reading_mode_enabled);
        std::printf("reading_type    : %d\n", s.reading_mode_type);
        std::printf("reading_ct      : %d\n", s.reading_mode_ct_level);
        std::printf("scheme_type     : %d\n", s.color_scheme_mode_type);
        std::printf("scheme_ct       : %d\n", s.color_scheme_ct_level);
        std::printf("backlight       : %d / %d\n",
                    s.backlight_current, s.backlight_max);
        return 0;
    }

    if (cmd == "readmode") {
        if (argc < 3) { usage(); return 1; }
        std::string v = argv[2];
        df_reading_mode_t m;
        if (v == "on")         m = DF_READING_PAPER;
        else if (v == "off")   m = DF_READING_OFF;
        else if (v == "paper") m = DF_READING_PAPER;
        else if (v == "soft")  m = DF_READING_SOFT;
        else { std::fprintf(stderr, "invalid mode\n"); return 1; }

        auto err = c.set_reading_mode(m);
        if (err) {
            std::fprintf(stderr, "set_reading_mode failed: %s\n",
                         err.message().c_str());
            return 3;
        }
        std::printf("reading mode set to %d\n", (int)m);
        return 0;
    }

    if (cmd == "backlight") {
        if (argc < 3) {
            auto r = c.get_backlight();
            if (r.ok()) std::printf("%d\n", r.value());
            return r.ok() ? 0 : 2;
        }
        int v = std::atoi(argv[2]);
        auto err = c.set_backlight(v);
        return err ? 3 : 0;
    }

    if (cmd == "probe") {
        int s = (argc > 2) ? std::atoi(argv[2]) : 1;
        int e = (argc > 3) ? std::atoi(argv[3]) : 40;
        auto r = c.probe(s, e);
        if (!r.ok()) { std::fprintf(stderr, "probe failed\n"); return 2; }
        for (auto& p : r.value()) {
            std::printf("code=%-3d perm=%d arg=%d ok=%d  %.60s\n",
                        p.code, p.perm_denied, p.arg_mismatch, p.ok,
                        p.raw.c_str());
        }
        return 0;
    }

    usage();
    return 1;
}
