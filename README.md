# DisplayFeature HAL Hook

Interposer `.so` untuk **Redmi 9T family** (lime/lemon/pomelo/citrus) yang
memungkinkan modifikasi perilaku `displayfeature.default.so` **tanpa build AOSP**.

## Cara kerja

Daripada build HAL dari source AOSP (butuh repo init + 100GB disk), kita:

1. **Rename** `.so` original Xiaomi → `.orig`
2. **Replace** dengan interposer kita di `/vendor/lib64/hw/`
3. Interposer `dlopen` file `.orig` dan **forward** semua call
4. Patch spesifik bisa dilakukan pada function pointer setelah layout struct diketahui

## Device yang didukung

| Codename | Model |
|---|---|
| `lime` | Redmi 9T Global / 9 Power |
| `lemon` | Redmi 9T China |
| `pomelo` | Redmi 9 Power India |
| `citrus` | POCO M3 |

Snapdragon 662 (SM6115), panel NT36672D.

## Build

### Via GitHub Actions

Push ke `main` → workflow otomatis build:
- `displayfeature.default-arm64-v8a.so`
- `displayfeature.default-armeabi-v7a.so`
- `DisplayFeatureHook.zip` (Magisk module)

Download dari tab **Actions** → **Artifacts**.

### Lokal

```bash
export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/28.0.13004108

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-30 \
  -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build -j
```

Hasil: `build/displayfeature.default.so`

## Instalasi

### Cara 1 — Magisk module (recommended)

```bash
# Download DisplayFeatureHook.zip dari Actions
adb push DisplayFeatureHook.zip /sdcard/

# Di device:
#   Buka Magisk Manager → Modules → Install from storage
#   Pilih DisplayFeatureHook.zip
#   Reboot
```

### Cara 2 — Manual

```bash
adb root
adb remount

# Backup original
adb shell su -c 'cp /vendor/lib64/hw/displayfeature.default.so \
    /vendor/lib64/hw/displayfeature.default.so.orig'

# Push interposer
adb push build/displayfeature.default.so \
    /vendor/lib64/hw/displayfeature.default.so

# Reboot
adb reboot
```

## Verifikasi

```bash
adb shell
su
logcat -c
stop vendor.displayfeature-hal-1-0
start vendor.displayfeature-hal-1-0
logcat -d | grep df-hook
```

Output yang diharapkan:

```
df-hook: ==============================================
df-hook: DisplayFeature interposer loaded
df-hook:   build   : Sep 29 2026 ...
df-hook:   version : 1.0.0
df-hook: ==============================================
df-hook: found original at: /vendor/lib64/hw/displayfeature.default.so.orig
df-hook: loading original: /vendor/lib64/hw/displayfeature.default.so.orig
df-hook: original loaded:
df-hook:   module   = 0x...
df-hook:   methods  = 0x...
df-hook:   open     = 0x...
df-hook:   id       = displayfeature
df-hook:   name     = DisplayFeature HAL
```

Kalau log ini muncul → interposer **berfungsi**.

## Analisa symbol (opsional)

Untuk analisa lebih dalam, jalankan:

```bash
# Tarik .so dari device
./scripts/pull_vendor_libs.sh

# Extract symbol
./scripts/extract_symbols.sh

# Lihat hasil
ls analyzed/
```

File `analyzed/lib64_displayfeature.default.dynsym.txt` berisi semua
symbol yang diekspor. Dari situ kita bisa tahu offset & layout struct.

## Uninstall

```bash
# Via Magisk Manager:
#   Modules → DisplayFeatureHook → Remove

# Manual:
adb shell su -c '
    rm -f /data/adb/modules/displayfeature_hook
    # Restore original
    cp /vendor/lib64/hw/displayfeature.default.so.orig \
       /vendor/lib64/hw/displayfeature.default.so
    reboot
'
```

## Lisensi

MIT

## Disclaimer

Project ini untuk **edukasi & riset pribadi**. Mengganti HAL pada device
Anda adalah tanggung jawab Anda sendiri. Pastikan punya backup TWRP
sebelum melakukan perubahan.
