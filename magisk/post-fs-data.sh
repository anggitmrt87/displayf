#!/system/bin/sh
MODDIR=${0%/*}

# Pastikan file backup original ada
ORIG64=/vendor/lib64/hw/displayfeature.default.so
ORIG32=/vendor/lib/hw/displayfeature.default.so

if [ -f "$ORIG64" ] && [ ! -f "${ORIG64}.orig" ]; then
    cp "$ORIG64" "${ORIG64}.orig"
    log -t displayfeature_hook "backed up $ORIG64"
fi

if [ -f "$ORIG32" ] && [ ! -f "${ORIG32}.orig" ]; then
    cp "$ORIG32" "${ORIG32}.orig"
    log -t displayfeature_hook "backed up $ORIG32"
fi

# Mount hook ke /vendor
if [ -f "$MODDIR/system/vendor/lib64/hw/displayfeature.default.so" ]; then
    mount -o bind \
        "$MODDIR/system/vendor/lib64/hw/displayfeature.default.so" \
        /vendor/lib64/hw/displayfeature.default.so
    log -t displayfeature_hook "mounted 64-bit hook"
fi

if [ -f "$MODDIR/system/vendor/lib/hw/displayfeature.default.so" ]; then
    mount -o bind \
        "$MODDIR/system/vendor/lib/hw/displayfeature.default.so" \
        /vendor/lib/hw/displayfeature.default.so
    log -t displayfeature_hook "mounted 32-bit hook"
fi
