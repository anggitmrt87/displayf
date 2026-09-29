#!/usr/bin/env bash
#
# extract_symbols.sh — analisa symbol dari .so vendor
#

set -e

LIBS="vendor_libs"
OUT="analyzed"
mkdir -p "$OUT"

if [ ! -d "$LIBS" ]; then
    echo "Jalankan pull_vendor_libs.sh dulu"
    exit 1
fi

# Cari nm
NM=$(which llvm-nm 2>/dev/null || \
     which aarch64-linux-gnu-nm 2>/dev/null || \
     which nm 2>/dev/null)
if [ -z "$NM" ]; then
    echo "nm tidak ditemukan"
    exit 1
fi

READELF=$(which llvm-readelf 2>/dev/null || which readelf)

for so in "$LIBS"/lib64/*.so "$LIBS"/lib/*.so; do
    [ -f "$so" ] || continue
    BASE=$(basename "$so" .so)
    DIR=$(basename $(dirname "$so"))
    TAG="${DIR}_${BASE}"

    echo "==> $TAG"

    # Dynamic symbols
    $NM -D --defined-only "$so" > "$OUT/${TAG}.dynsym.txt" 2>/dev/null || true
    $NM -D --undefined-only "$so" > "$OUT/${TAG}.undef.txt" 2>/dev/null || true

    # Dynamic deps
    if [ -n "$READELF" ]; then
        $READELF -dW "$so" 2>/dev/null | grep NEEDED > "$OUT/${TAG}.deps.txt" || true
    fi

    # Strings menarik
    strings -n 4 "$so" | \
        grep -iE "displayfeature|HIDL_FETCH|HAL_MODULE|hw_module|df_device|setFeature|DisplayEffect|DisplayFeatureHal" | \
        sort -u > "$OUT/${TAG}.strings.txt" || true

    # Jumlah
    DYN=$(wc -l < "$OUT/${TAG}.dynsym.txt" 2>/dev/null || echo 0)
    UND=$(wc -l < "$OUT/${TAG}.undef.txt" 2>/dev/null || echo 0)
    echo "  dynsym: $DYN, undef: $UND"
done

echo
echo "==> Done. Output di $OUT/"
ls -la "$OUT/"
