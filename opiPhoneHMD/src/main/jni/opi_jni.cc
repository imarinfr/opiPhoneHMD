/*
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#include <jni.h>

#include <memory>

#include "opi_app.h"

#define JNI_METHOD_MAIN_OPILAUNCHER(return_type, method_name) \
  JNIEXPORT return_type JNICALL                           \
            Java_com_optocom_imarinfr_opi_Main_##method_name

#define JNI_METHOD_OPICONNECTION(return_type, method_name) \
  JNIEXPORT return_type JNICALL                            \
            Java_com_optocom_imarinfr_opi_OpiConnection_##method_name

#define JNI_METHOD_OPIRENDERER(return_type, method_name) \
  JNIEXPORT return_type JNICALL                          \
            Java_com_optocom_imarinfr_opi_Renderer_##method_name

namespace {

    inline jlong jptr(ndk_opi::OpiApp* native_app) {
        return reinterpret_cast<intptr_t>(native_app);
    }

    inline ndk_opi::OpiApp* native(jlong ptr) {
        return reinterpret_cast<ndk_opi::OpiApp*>(ptr);
    }

    JavaVM* javaVm;

}  // anonymous namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

// Native methods for Main
JNI_METHOD_MAIN_OPILAUNCHER(jlong, nativeOnCreate)
(JNIEnv* /*env*/, jobject obj) {
    return jptr(new ndk_opi::OpiApp(javaVm, obj));
}

JNI_METHOD_MAIN_OPILAUNCHER(void, nativeOnDestroy)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app) {
    delete native(native_app);
}

JNI_METHOD_MAIN_OPILAUNCHER(void, nativeOnPause)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app) {
    native(native_app)->OnPause();
}

JNI_METHOD_MAIN_OPILAUNCHER(void, nativeOnResume)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app) {
    native(native_app)->OnResume();
}

JNI_METHOD_MAIN_OPILAUNCHER(void, nativeOnSwitchViewer)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app) {
    native(native_app)->SwitchViewer();
}

// Native methods for OpiConnection
JNI_METHOD_OPICONNECTION(jfloatArray, nativeGetFieldOfView)
(JNIEnv* env, jobject /*obj*/, jlong native_app) {
    jfloatArray fov = native(native_app)->returnFieldOfView(env);
    return fov;
}

// Native methods for Renderer
JNI_METHOD_OPIRENDERER(void, nativeOnSurfaceCreated)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app) {
    native(native_app)->OnSurfaceCreated();
}

JNI_METHOD_OPIRENDERER(void, nativeSetScreenParams)
(JNIEnv* /*env*/, jobject /*obj*/, jlong native_app, jint width, jint height) {
    native(native_app)->SetScreenParams(width, height);
}

JNI_METHOD_OPIRENDERER(void, nativeOnDrawFrame)
(JNIEnv* env, jobject /*obj*/, jlong native_app, jint bgeye, jfloat bglum,
 jfloatArray bgcol, jint fixeye, jint fixtype, jfloat fixcx, jfloat fixcy, jfloat fixsx,
 jfloat fixsy, jfloat fixtheta, jfloat fixlum, jfloatArray fixcol, jint steye,
 jint sttype, jfloat stcx, jfloat stcy, jfloat stsx, jfloat stsy,
 jfloat sttheta, jfloat stlum, jfloatArray stcol) {
    native(native_app)->OnDrawFrame(env, bgeye, bglum, bgcol, fixeye, fixtype, fixcx, fixcy,
                                    fixsx, fixsy, fixtheta, fixlum, fixcol, steye, sttype, stcx,
                                    stcy, stsx, stsy, sttheta, stlum, stcol);
}

}  // extern "C"