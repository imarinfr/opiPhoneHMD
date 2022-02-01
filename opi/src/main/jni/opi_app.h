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

#ifndef OPI_ANDROID_SRC_MAIN_JNI_OPI_APP_H_
#define OPI_ANDROID_SRC_MAIN_JNI_OPI_APP_H_

#include <jni.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <GLES2/gl2.h>
#include "cardboard.h"
#include "util.h"

namespace ndk_opi {

/**
 * This is a sample opi for the Cardboard SDK. It loads a simple environment and
 * objects that you can click on.
 */
    class OpiApp {
    public:
        /**
         * Creates a OpiApp.
         *
         * @param vm JavaVM pointer.
         * @param obj Android activity object.
         */
        OpiApp(JavaVM* vm, jobject obj);

        ~OpiApp();

        /**
         * Initializes any GL-related objects. This should be called on the rendering
         * thread with a valid GL context.
         */
        void OnSurfaceCreated();

        /**
         * Sets screen parameters.
         *
         * @param width Screen width
         * @param height Screen height
         */
        void SetScreenParams(int width, int height);

        /**
         * Draws the frame
         */
        void OnDrawFrame(JNIEnv* env, jint bgeye, jfloat bglum, jfloatArray bgc, jint fixeye,
                         jint fixtype, jfloat fixcx, jfloat fixcy, jfloat fixsx, jfloat fixsy,
                         jfloat fixtheta, jfloat fixlum, jfloatArray fixc, jint steye,
                         jint sttype, jfloat stcx, jfloat stcy, jfloat stsx, jfloat stsy,
                         jfloat sttheta, jfloat stlum, jfloatArray stc);

        /**
         * Pauses head tracking.
         */
        void OnPause();

        /**
         * Resumes head tracking.
         */
        void OnResume();

        /**
         * Allows user to switch viewer.
         */
        void SwitchViewer();

        /**
         * Retrieves the field of view form the cardboard viewer
         */
        void getFieldOfView();
        /**
         * Returns the field of view of the cardboard viewer to Java
         */
        jfloatArray returnFieldOfView(JNIEnv* env);

    private:
        /**
         * Default near clip plane z-axis coordinate.
         */
        static constexpr float kZNear = 1.0f;

        /**
         * Default far clip plane z-axis coordinate.
         */
        static constexpr float kZFar = 50.f;

        /**
         * Updates device parameters, if necessary.
         *
         * @return true if device parameters were successfully updated.
         */
        bool UpdateDeviceParams();

        /**
         * Initializes GL environment.
         */
        void GlSetup();

        /**
         * Deletes GL environment.
         */
        void GlTeardown();

        /**
         * This should be called on the rendering thread
         */
        bool PrepareBuffer();

        /**
         * Draws the background.
         */
        void DrawBackground(float lum, float col[]);

        /**
         * Draws the fixation target.
         */
        void DrawFixationTarget(float cx, float cy, float sx, float sy, float theta,
                                float lum, float col[]);

        /**
         * Draws the stimulus.
         */
        void DrawStimulus(float cx, float cy, float rx, float ry, float theta,
                          float lum, float col[]);

        /**
         * Gets shape code
        */
        Shape getShapeCode(int type);

        /**
         * Converts from degrees to radians
        */
        static float RadiansToDegrees(float angle);

        /**
         * Converts from degrees to radians
         */
        static float DegreesToRadians(float angle);

        /**
         * Converts Obtains the Height in meters from distances and angle of view
         */
        static float degOfViewToLength(int distance, float angle);

        CardboardLensDistortion* lens_distortion;
        CardboardDistortionRenderer* distortion_renderer;

        CardboardEyeTextureDescription left_eye_texture_description;
        CardboardEyeTextureDescription right_eye_texture_description;

        bool screen_params_changed;
        bool device_params_changed;
        int screen_width;
        int screen_height;

        float projection_matrices[2][16]{};
        float eye_matrices[2][16]{};

        GLuint depthRenderBuffer;  // depth buffer
        GLuint framebuffer;        // framebuffer object
        GLuint texture;            // distortion texture

        GLuint obj_program;

        Matrix4x4 modelview_background{};
        Matrix4x4 modelview_fixation_target{};
        Matrix4x4 modelview_stimulus{};

        Shape nothing;
        Shape circle;
        Shape square;
        Shape cross;
        Shape maltese;
        Shape annulus;
        Shape background;
        Shape fixation;
        Shape stimulus;

        float fov[4] = {45, 45, 45, 45};
    };

}  // namespace ndk_opi

#endif  // OPI_ANDROID_SRC_MAIN_JNI_OPI_APP_H_
