#!/usr/bin/env bash
#
# package_magisk.sh — package .so interposer jadi Magisk module zip
#
# Usage: ./package_magisk.sh <artifacts-dir>
#   artifacts-dir/arm64/displayfeature.default-arm64-v8a.so
#   artifacts-dir/arm32/displayfeature.default-armeabi-v7a.so
#

set -e

ARTIFACTS_DIR="${1:?Usage: $0 <artifacts-dir>}"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

MOD="$WORK/DisplayFeatureHook"
mkdir -p "$MOD/system/vendor/lib64/hw"
mkdir -p "$MOD/system/vendor/lib/hw"

# Copy library
ARM64_SO="$ARTIFACTS_DIR/arm64/displayfeature.default-arm64-v8a.so"
ARM32_SO="$ARTIFACTS_DIR/arm32/displayfeature.default-armeabi-v7a.so"

if [ -f "$ARM64_SO" ]; then
    cp "$ARM64_SO" "$MOD/system/vendor/lib64/hw/displayfeature.default.so"
    echo "  + lib64: $(basename $ARM64_SO)"
else
    echo "WARN: $ARM64_SO not found"
fi

if [ -f "$ARM32_SO" ]; then
    cp "$ARM32_SO" "$MOD/system/vendor/lib/hw/displayfeature.default.so"
    echo "  + lib32: $(basename $ARM32_SO)"
fi

# Copy Magisk module files
cp magisk/module.prop       "$MOD/"
cp magisk/customize.sh      "$MOD/"
cp magisk/post-fs-data.sh   "$MOD/"
cp magisk/service.sh        "$MOD/"

chmod 755 "$MOD/customize.sh" "$MOD/post-fs-data.sh" "$MOD/service.sh"

# Zip
cd "$WORK"
zip -r9 "$OLDPWD/DisplayFeatureHook.zip" DisplayFeatureHook/*

echo
echo "==> Package: $OLDPWD/DisplayFeatureHook.zip"
ls -la "$OLDPWD/DisplayFeatureHook.zip"
