#ifdef __ANDROID__
#include <jni.h>
#include <string>

#include "displayfeature/client.h"

using namespace displayfeature;

static std::string jstr(JNIEnv* env, jstring s) {
    if (!s) return {};
    const char* c = env->GetStringUTFChars(s, nullptr);
    std::string r = c ? c : "";
    if (c) env->ReleaseStringUTFChars(s, c);
    return r;
}

static jstring to_jstr(JNIEnv* env, const std::string& s) {
    return env->NewStringUTF(s.c_str());
}

extern "C" {

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeCreate(
        JNIEnv*, jclass) {
    auto* c = new Client();
    auto e = c->open();
    if (e) { delete c; return 0; }
    return reinterpret_cast<jint>((intptr_t)c);
}

JNIEXPORT void JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeDestroy(
        JNIEnv*, jclass, jint handle) {
    delete reinterpret_cast<Client*>((intptr_t)handle);
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeSetReadingMode(
        JNIEnv*, jclass, jint handle, jint mode) {
    auto c = reinterpret_cast<Client*>((intptr_t)handle);
    if (!c) return DF_ERR_INVALID_ARG;
    return c->set_reading_mode((df_reading_mode_t)mode).code();
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeSetBacklight(
        JNIEnv*, jclass, jint handle, jint value) {
    auto c = reinterpret_cast<Client*>((intptr_t)handle);
    if (!c) return DF_ERR_INVALID_ARG;
    return c->set_backlight(value).code();
}

JNIEXPORT jint JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeGetBacklight(
        JNIEnv*, jclass, jint handle) {
    auto c = reinterpret_cast<Client*>((intptr_t)handle);
    if (!c) return -1;
    auto r = c->get_backlight();
    return r.ok() ? r.value() : -1;
}

JNIEXPORT jstring JNICALL
Java_com_example_displayfeature_DisplayFeatureNative_nativeDumpsys(
        JNIEnv* env, jclass, jint handle) {
    auto c = reinterpret_cast<Client*>((intptr_t)handle);
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
