#ifdef __ANDROID__
#include <jni.h>
#include <cstdint>
#include <string>

#include "displayfeature/client.h"

using namespace displayfeature;

/* ===================================================================
 * Helper
 * =================================================================== */

namespace {

std::string jstr(JNIEnv* env, jstring s) {
    if (!s) return {};
    const char* c = env->GetStringUTFChars(s, nullptr);
    std::string r = c ? c : "";
    if (c) env->ReleaseStringUTFChars(s, c);
    return r;
}

jstring to_jstr(JNIEnv* env, const std::string& s) {
    return env->NewStringUTF(s.c_str());
}

/* Handle: pointer Client disimpan sebagai jlong (64-bit).
 * Jangan pakai jint (32-bit) — pointer 64-bit akan terpotong. */
inline jlong to_handle(Client* c) {
    return static_cast<jlong>(reinterpret_cast<intptr_t>(c));
}

inline Client* from_handle(jlong h) {
    return reinterpret_cast<Client*>(static_cast<intptr_t>(h));
}

} /* anonymous namespace */

/* ===================================================================
 * JNI methods
 * =================================================================== */

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeCreate(
        JNIEnv*, jclass) {
    auto* c = new (std::nothrow) Client();
    if (!c) return 0;
    auto e = c->open();
    if (e) {
        delete c;
        return 0;
    }
    return to_handle(c);
}

JNIEXPORT void JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeDestroy(
        JNIEnv*, jclass, jlong handle) {
    delete from_handle(handle);
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeSetReadingMode(
        JNIEnv*, jclass, jlong handle, jint mode) {
    auto c = from_handle(handle);
    if (!c) return DF_ERR_INVALID_ARG;
    return c->set_reading_mode(static_cast<df_reading_mode_t>(mode)).code();
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeSetBacklight(
        JNIEnv*, jclass, jlong handle, jint value) {
    auto c = from_handle(handle);
    if (!c) return DF_ERR_INVALID_ARG;
    return c->set_backlight(value).code();
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeGetBacklight(
        JNIEnv*, jclass, jlong handle) {
    auto c = from_handle(handle);
    if (!c) return -1;
    auto r = c->get_backlight();
    return r.ok() ? r.value() : -1;
}

JNIEXPORT jstring JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeDumpsys(
        JNIEnv* env, jclass, jlong handle) {
    auto c = from_handle(handle);
    if (!c) return to_jstr(env, "");
    auto r = c->dumpsys();
    return to_jstr(env, r.ok() ? r.value() : std::string());
}

JNIEXPORT jboolean JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeIsSupportedDevice(
        JNIEnv* env, jclass, jstring codename) {
    std::string c = jstr(env, codename);
    return Client::is_supported_device(&c) ? JNI_TRUE : JNI_FALSE;
}

} /* extern "C" */

#endif /* __ANDROID__ */
