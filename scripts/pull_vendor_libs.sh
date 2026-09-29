#!/usr/bin/env bash
#
# pull_vendor_libs.sh — tarik .so dari device ke folder lokal
#

set -e

OUT="vendor_libs"
mkdir -p "$OUT/lib64" "$OUT/lib"

if ! command -v adb &>/dev/null; then
    echo "adb tidak ditemukan"
    exit 1
fi

if ! adb devices | grep -q "device$"; then
    echo "Tidak ada device terhubung"
    exit 1
fi

FILES_64=(
    /vendor/lib64/hw/displayfeature.default.so
    /vendor/lib64/hw/vendor.xiaomi.hardware.displayfeature@1.0-impl.so
    /vendor/lib64/vendor.xiaomi.hardware.displayfeature@1.0.so
    /vendor/lib64/libdisplayfeature.so
    /vendor/lib64/libdisplayfeatureservice.so
    /vendor/lib64/libhidlbase.so
    /vendor/lib64/libhidltransport.so
    /vendor/lib64/libbinder.so
    /vendor/lib64/libutils.so
    /vendor/lib64/libbase.so
    /vendor/lib64/libcutils.so
    /vendor/bin/hw/vendor.xiaomi.hardware.displayfeature@1.0-service
)

FILES_32=(
    /vendor/lib/hw/displayfeature.default.so
    /vendor/lib/hw/vendor.xiaomi.hardware.displayfeature@1.0-impl.so
    /vendor/lib/vendor.xiaomi.hardware.displayfeature@1.0.so
    /vendor/lib/libdisplayfeature.so
    /vendor/lib/libdisplayfeatureservice.so
)

echo "==> Pull 64-bit files"
for f in "${FILES_64[@]}"; do
    base=$(basename "$f")
    if adb pull "$f" "$OUT/lib64/$base" 2>/dev/null; then
        echo "  OK  $base"
    else
        echo "  --  $base (not found)"
    fi
done

echo "==> Pull 32-bit files"
for f in "${FILES_32[@]}"; do
    base=$(basename "$f")
    if adb pull "$f" "$OUT/lib/$base" 2>/dev/null; then
        echo "  OK  $base"
    else
        echo "  --  $base (not found)"
    fi
done

echo
echo "==> Done. Files in: $OUT/"
du -sh "$OUT"
