#include "AudioEngine.h"
#include <android/log.h>

#define LOG_TAG "AudioEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace RacingEngine {

AudioEngine::AudioEngine()
    : javaVM(nullptr), audioHelper(nullptr),
      playSoundMethod(nullptr), updateEngineMethod(nullptr),
      stopEngineMethod(nullptr), initialized(false) {}

AudioEngine::~AudioEngine() {
    shutdown();
}

void AudioEngine::initialize(JavaVM* vm, jobject context) {
    if (initialized) return;

    javaVM = vm;
    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    // Find AudioHelper class
    jclass audioHelperClass = env->FindClass("co/za/chefu/racingdominion/AudioHelper");
    if (audioHelperClass == nullptr) {
        LOGE("Could not find AudioHelper class");
        if (needsDetach) javaVM->DetachCurrentThread();
        return;
    }

    // Get constructor
    jmethodID constructor = env->GetMethodID(audioHelperClass, "<init>", "(Landroid/content/Context;)V");
    if (constructor == nullptr) {
        LOGE("Could not find AudioHelper constructor");
        if (needsDetach) javaVM->DetachCurrentThread();
        return;
    }

    // Create instance
    jobject helper = env->NewObject(audioHelperClass, constructor, context);
    audioHelper = env->NewGlobalRef(helper);

    // Get methods
    playSoundMethod = env->GetMethodID(audioHelperClass, "playSound", "(I)V");
    updateEngineMethod = env->GetMethodID(audioHelperClass, "updateEngineSound", "(F)V");
    stopEngineMethod = env->GetMethodID(audioHelperClass, "stopEngineSound", "()V");

    initialized = true;
    LOGI("Audio engine initialized");

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::shutdown() {
    if (!initialized) return;

    if (javaVM && audioHelper) {
        JNIEnv* env;
        bool needsDetach = false;

        jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (result == JNI_EDETACHED) {
            javaVM->AttachCurrentThread(&env, nullptr);
            needsDetach = true;
        }

        if (stopEngineMethod) {
            env->CallVoidMethod(audioHelper, stopEngineMethod);
        }

        env->DeleteGlobalRef(audioHelper);
        audioHelper = nullptr;

        if (needsDetach) javaVM->DetachCurrentThread();
    }

    initialized = false;
    LOGI("Audio engine shutdown");
}

void AudioEngine::playEngineSound(float speed) {
    if (!initialized || !updateEngineMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, updateEngineMethod, (jfloat)speed);

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::stopEngineSound() {
    if (!initialized || !stopEngineMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, stopEngineMethod);

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::playBrakeSound() {
    if (!initialized || !playSoundMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, playSoundMethod, (jint)1);

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::playCollisionSound() {
    if (!initialized || !playSoundMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, playSoundMethod, (jint)2);

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::playCountdownBeep(int number) {
    if (!initialized || !playSoundMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, playSoundMethod, (jint)(3 + number));

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::playFinishSound() {
    if (!initialized || !playSoundMethod) return;

    JNIEnv* env;
    bool needsDetach = false;

    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        javaVM->AttachCurrentThread(&env, nullptr);
        needsDetach = true;
    }

    env->CallVoidMethod(audioHelper, playSoundMethod, (jint)6);

    if (needsDetach) javaVM->DetachCurrentThread();
}

void AudioEngine::setMasterVolume(float volume) {
    // Volume is handled on the Java side
}

void AudioEngine::setSfxVolume(float volume) {
    // Volume is handled on the Java side
}

} // namespace RacingEngine
