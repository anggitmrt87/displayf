#!/system/bin/sh
MODDIR=${0%/*}

# Tunggu boot selesai
until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 1
done

sleep 3

# Restart HAL service supaya load .so yang baru
log -t displayfeature_hook "restarting HAL service"

stop vendor.displayfeature-hal-1-0 2>/dev/null
sleep 1
start vendor.displayfeature-hal-1-0 2>/dev/null

# Cek apakah service jalan
sleep 2
if pidof vendor.xiaomi.hardware.displayfeature@1.0-service > /dev/null; then
    log -t displayfeature_hook "HAL service running"
else
    log -t displayfeature_hook "WARNING: HAL service not running"
fi
