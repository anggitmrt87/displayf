#!/system/bin/sh

SKIPUNZIP=0

ui_print "*********************************"
ui_print " DisplayFeature HAL Hook"
ui_print "*********************************"
ui_print ""

DEVICE=$(getprop ro.product.device)
case "$DEVICE" in
    lime|lemon|pomelo|citrus)
        ui_print "- Device: $DEVICE"
        ui_print "- Supported device detected"
        ;;
    *)
        ui_print "! Unsupported device: $DEVICE"
        ui_print "! Only for Redmi 9T family"
        abort "! Aborting installation"
        ;;
esac

# Backup original .so (jika belum ada backup)
ORIG64=/vendor/lib64/hw/displayfeature.default.so
ORIG32=/vendor/lib/hw/displayfeature.default.so

if [ -f "$ORIG64" ] && [ ! -f "${ORIG64}.orig" ]; then
    ui_print "- Backing up: $ORIG64"
    cp "$ORIG64" "${ORIG64}.orig"
fi

if [ -f "$ORIG32" ] && [ ! -f "${ORIG32}.orig" ]; then
    ui_print "- Backing up: $ORIG32"
    cp "$ORIG32" "${ORIG32}.orig"
fi

ui_print "- Installation complete"
ui_print "- Reboot to activate"