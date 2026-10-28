#include <jni.h>
#include <android/asset_manager_jni.h>
#include "Engine.h"

using namespace RacingEngine;
static Engine* engine = nullptr;
extern "C" {
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeInit(JNIEnv* env, jobject, jobject assets) {
    // Surface recreation begins a fresh session; no stale engine survives a lost GL context.
    delete engine;
    engine = new Engine();
    if (!engine->initialize(AAssetManager_fromJava(env, assets))) {
        delete engine; engine = nullptr;
        jclass error = env->FindClass("java/lang/IllegalStateException");
        if (error) env->ThrowNew(error, "Racing Dominion could not initialize OpenGL ES 3.0.");
    }
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeResize(JNIEnv*, jobject, jint w, jint h) {
    if (engine) engine->resize(w, h);
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeUpdate(JNIEnv*, jobject, jfloat dt) {
    if (engine) engine->update(dt);
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeRender(JNIEnv*, jobject) {
    if (engine) engine->render();
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeSetControls(JNIEnv*, jobject, jfloat steer, jfloat gas, jfloat brake) {
    if (engine) engine->setControls(steer, gas, brake);
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeAction(JNIEnv*, jobject, jint action) {
    if (engine) engine->action(action);
}
JNIEXPORT void JNICALL Java_co_za_chefu_racingdominion_RacingRenderer_nativeReadTelemetry(JNIEnv* env, jobject, jfloatArray output) {
    if (!engine || env->GetArrayLength(output) < 34) return;
    float values[34]{};
    engine->writeTelemetry(values);
    env->SetFloatArrayRegion(output, 0, 34, values);
}
}
