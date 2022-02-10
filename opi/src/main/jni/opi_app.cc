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

#include "opi_app.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <array>
#include <cmath>
#include <fstream>

#include "cardboard.h"

namespace ndk_opi {

    namespace {

// Background distance is far away, fixation target ought to be closer,
// and stimulus just behind fixation target
        constexpr float kBackgroundDistance     = 50.0f;
        constexpr float kFixationTargetDistance = 40.0f;
        constexpr float kStimulusDistance       = 45.0f;

        Matrix4x4 projection_matrix;
        Matrix4x4 model_background;
        Matrix4x4 model_fixation_target;
        Matrix4x4 model_stimulus;

// Simple shaders to render .obj files without any lighting.
        constexpr const char* kObjVertexShader =
                R"glsl(
    uniform mat4 u_MVP;
    attribute vec4 a_Position;
    attribute vec2 a_UV;
    varying vec2 v_UV;

    void main() {
      v_UV = a_UV;
      gl_Position = u_MVP * a_Position;
    })glsl";

        constexpr const char* kObjFragmentShader =
                R"glsl(
    precision mediump float;

    uniform sampler2D u_Texture;
    varying vec2 v_UV;

    void main() {
      // The y coordinate of this sample's textures is reversed compared to
      // what OpenGL expects, so we invert the y coordinate.
      gl_FragColor = texture2D(u_Texture, vec2(v_UV.x, 1.0 - v_UV.y));
    })glsl";

    }  // anonymous namespace

    OpiApp::OpiApp(JavaVM* vm, jobject obj)
            : lens_distortion(nullptr),
              distortion_renderer(nullptr),
              left_eye_texture_description({}),
              right_eye_texture_description({}),
              screen_params_changed(false),
              device_params_changed(false),
              screen_width(0),
              screen_height(0),
              depthRenderBuffer(0),
              framebuffer(0),
              texture(0),
              obj_program(0) {
      JNIEnv* env;
      vm->GetEnv((void**)&env, JNI_VERSION_1_6);

      Cardboard_initializeAndroid(vm, obj);
    }

    OpiApp::~OpiApp() {
      CardboardLensDistortion_destroy(lens_distortion);
      CardboardDistortionRenderer_destroy(distortion_renderer);
    }

    void OpiApp::OnSurfaceCreated() {
      const GLuint obj_vertex_shader = LoadGLShader(GL_VERTEX_SHADER, kObjVertexShader);
      const GLuint obj_fragment_shader = LoadGLShader(GL_FRAGMENT_SHADER, kObjFragmentShader);

      obj_program = glCreateProgram();
      glAttachShader(obj_program, obj_vertex_shader);
      glAttachShader(obj_program, obj_fragment_shader);
      glLinkProgram(obj_program);
      glUseProgram(obj_program);

      CHECKGLERROR("Obj program");

      GLuint obj_pos = glGetAttribLocation(obj_program, "a_Position");
      GLuint obj_uv  = glGetAttribLocation(obj_program, "a_UV");
      CHECKGLERROR("Obj program params");

      // generate shapes
      nothing.Initialize(obj_pos, obj_uv, Shape::NONE);
      circle.Initialize(obj_pos, obj_uv,  Shape::CIRCLE);
      square.Initialize(obj_pos, obj_uv,  Shape::SQUARE);
      cross.Initialize(obj_pos, obj_uv,   Shape::CROSS);
      maltese.Initialize(obj_pos, obj_uv, Shape::MALTESE);
      annulus.Initialize(obj_pos, obj_uv, Shape::ANNULUS);
      // initialize a shape to use for background, fixation target, and stimulus
      background = square;
      fixation   = nothing;
      stimulus   = nothing;
      CHECKGLERROR("OnSurfaceCreated");
    }

    void OpiApp::SetScreenParams(int width, int height) {
      screen_width = width;
      screen_height = height;
      screen_params_changed = true;
    }

    bool OpiApp::PrepareBuffer() {
      if(UpdateDeviceParams()) {
        // Bind buffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }
      return UpdateDeviceParams();
    }

    void OpiApp::OnDrawFrame(JNIEnv* env, jint bgeye, jfloat bglum, jfloatArray bgc, jint fixeye,
                             jint fixtype, jfloat fixcx, jfloat fixcy, jfloat fixsx, jfloat fixsy,
                             jfloat fixtheta, jfloat fixlum, jfloatArray fixc, jint steye,
                             jint sttype, jfloat stcx, jfloat stcy, jfloat stsx, jfloat stsy,
                             jfloat sttheta, jfloat stlum, jfloatArray stc) {
      if(!PrepareBuffer()) return;
      fixation = getShapeCode(fixtype); // get fixation object
      stimulus = getShapeCode(sttype);  // get stimulus object
      // get background, fixation, and stimulus targets
      jfloat bgcol[3]; jfloat fixcol[3]; jfloat stcol[3];
      env->GetFloatArrayRegion(bgc, 0, 3, bgcol);
      env->GetFloatArrayRegion(fixc, 0, 3, fixcol);
      env->GetFloatArrayRegion(stc, 0, 3, stcol);
      for (int eye = 0; eye < 2; ++eye) { // Draw eyes views
        glViewport(eye == kLeft ? 0 : screen_width / 2, 0, screen_width / 2, screen_height);
        projection_matrix = GetMatrixFromGlArray(projection_matrices[eye]);
        if(bgeye  == eye || bgeye  == 2) DrawBackground(bglum, bgcol);
        if(fixeye == eye || fixeye == 2) DrawFixationTarget(fixcx, fixcy, fixsx, fixsy, fixtheta, fixlum, fixcol);
        if(steye  == eye || steye  == 2) DrawStimulus(stcx, stcy, stsx, stsy, sttheta, stlum, stcol);
      }
      // Render
      CardboardDistortionRenderer_renderEyeToDisplay(
              distortion_renderer, /* target = */ 0, /* x = */ 0, /* y = */ 0,
              screen_width, screen_height, &left_eye_texture_description,
              &right_eye_texture_description);
    }

    void OpiApp::OnPause() {
    }

    void OpiApp::OnResume() {
      // Parameters may have changed.
      device_params_changed = true;

      // Check for device parameters existence in external storage. If they're
      // missing, we must scan a Cardboard QR code and save the obtained parameters.
      uint8_t* buffer;
      int size;
      CardboardQrCode_getSavedDeviceParams(&buffer, &size);
      if (size == 0) {
        SwitchViewer();
      }
      CardboardQrCode_destroy(buffer);
    }

    void OpiApp::SwitchViewer() { // NOLINT(readability-convert-member-functions-to-static)
      CardboardQrCode_scanQrCodeAndSaveDeviceParams();
    }

    bool OpiApp::UpdateDeviceParams() {
      // Checks if screen or device parameters changed
      if (!screen_params_changed && !device_params_changed) {
        return true;
      }

      // Get saved device parameters
      uint8_t* buffer;
      int size;
      CardboardQrCode_getSavedDeviceParams(&buffer, &size);

      // If there are no parameters saved yet, returns false.
      if (size == 0) {
        return false;
      }

      CardboardLensDistortion_destroy(lens_distortion);
      lens_distortion = CardboardLensDistortion_create(buffer, size, screen_width, screen_height);

      CardboardQrCode_destroy(buffer);

      GlSetup();

      CardboardDistortionRenderer_destroy(distortion_renderer);
      distortion_renderer = CardboardOpenGlEs2DistortionRenderer_create();

      // Setup mesh for left and right eye
      CardboardMesh left_mesh;
      CardboardMesh right_mesh;
      CardboardLensDistortion_getDistortionMesh(lens_distortion, kLeft,  &left_mesh);
      CardboardLensDistortion_getDistortionMesh(lens_distortion, kRight, &right_mesh);
      CardboardDistortionRenderer_setMesh(distortion_renderer, &left_mesh,  kLeft);
      CardboardDistortionRenderer_setMesh(distortion_renderer, &right_mesh, kRight);

      // Get eye matrices
      CardboardLensDistortion_getEyeFromHeadMatrix(lens_distortion, kLeft,  eye_matrices[0]);
      CardboardLensDistortion_getEyeFromHeadMatrix(lens_distortion, kRight, eye_matrices[1]);
      CardboardLensDistortion_getProjectionMatrix(lens_distortion, kLeft, kZNear, kZFar, projection_matrices[0]);
      CardboardLensDistortion_getProjectionMatrix(lens_distortion, kRight, kZNear, kZFar, projection_matrices[1]);

      screen_params_changed = false;
      device_params_changed = false;

      CHECKGLERROR("UpdateDeviceParams");

      getFieldOfView();

      return true;
    }

    void OpiApp::GlSetup() {
      LOGD("GL SETUP");

      if (framebuffer != 0) {
        GlTeardown();
      }

      // Create render texture.
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

      left_eye_texture_description.texture = texture;
      left_eye_texture_description.left_u = 0;
      left_eye_texture_description.right_u = 0.5;
      left_eye_texture_description.top_v = 1;
      left_eye_texture_description.bottom_v = 0;

      right_eye_texture_description.texture = texture;
      right_eye_texture_description.left_u = 0.5;
      right_eye_texture_description.right_u = 1;
      right_eye_texture_description.top_v = 1;
      right_eye_texture_description.bottom_v = 0;

      // Generate depth buffer to perform depth test.
      glGenRenderbuffers(1, &depthRenderBuffer);
      glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screen_width, screen_height);
      CHECKGLERROR("Create Render buffer");

      // Create render target.
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

      CHECKGLERROR("GlSetup");
    }

    void OpiApp::GlTeardown() {
      glDeleteRenderbuffers(1, &depthRenderBuffer);
      depthRenderBuffer = 0;
      glDeleteFramebuffers(1, &framebuffer);
      framebuffer = 0;
      glDeleteTextures(1, &texture);
      texture = 0;

      CHECKGLERROR("GlTeardown");
    }

    void OpiApp::getFieldOfView() {
        // left eye mirror image of left eye, so only retrieve right
        float *fov_ptr;
        fov_ptr = fov;
        CardboardLensDistortion_getFieldOfView(lens_distortion, kRight, fov_ptr);
        for(float & i : fov) if(i == DegreesToRadians(45.0f)) i = - 1.0f;
    }

    jfloatArray OpiApp::returnFieldOfView(JNIEnv* env) {
      jfloatArray jfov = env->NewFloatArray(4);
      float fovd[4] = {RadiansToDegrees(fov[0]),
                       RadiansToDegrees(fov[1]),
                       RadiansToDegrees(fov[2]),
                       RadiansToDegrees(fov[3])};
      env->SetFloatArrayRegion(jfov, 0, 4, fovd);
      return jfov;
    }

    void OpiApp::DrawBackground(float lum, float col[]) {
      glUseProgram(obj_program);
      float sx = degOfViewToLength(kBackgroundDistance, std::max(fov[0], fov[1]));
      float sy = degOfViewToLength(kBackgroundDistance, std::max(fov[0], fov[1]));
      model_background = GetAffineMatrix(sx, sy, 0, {0, 0, kBackgroundDistance});
      modelview_background = projection_matrix * model_background;
      std::array<float, 16> position = modelview_background.ToGlArray();
      glUniformMatrix4fv(0, 1, GL_FALSE, position.data());
      background.Ready(lum, col);
      background.Draw();
      CHECKGLERROR("DrawBackground");
    }

    void OpiApp:: DrawFixationTarget(float cx, float cy, float sx, float sy, float theta,
                                     float lum, float col[]) {
      glUseProgram(obj_program);
      sx = degOfViewToLength(kFixationTargetDistance, DegreesToRadians(sx));
      sy = degOfViewToLength(kFixationTargetDistance, DegreesToRadians(sy));
      cx = degOfViewToLength(kFixationTargetDistance, DegreesToRadians(cx));
      cy = degOfViewToLength(kFixationTargetDistance, DegreesToRadians(cy));
      model_fixation_target = GetAffineMatrix(sx, sy, DegreesToRadians(theta), {cx, cy, kFixationTargetDistance});
      modelview_fixation_target = projection_matrix * model_fixation_target;
      std::array<float, 16> position = modelview_fixation_target.ToGlArray();
      glUniformMatrix4fv(0, 1, GL_FALSE, position.data());
      fixation.Ready(lum, col);
      fixation.Draw();
      CHECKGLERROR("DrawFixationTarget");
    }

    void OpiApp::DrawStimulus(float cx, float cy,  float sx, float sy, float theta,
                              float lum, float col[]) {
      glUseProgram(obj_program);
      sx = degOfViewToLength(kStimulusDistance, DegreesToRadians(sx));
      sy = degOfViewToLength(kStimulusDistance, DegreesToRadians(sy));
      cx = degOfViewToLength(kStimulusDistance, DegreesToRadians(cx));
      cy = degOfViewToLength(kStimulusDistance, DegreesToRadians(cy));
      model_stimulus     = GetAffineMatrix(sx, sy, DegreesToRadians(theta), {cx, cy, kStimulusDistance});
      modelview_stimulus = projection_matrix * model_stimulus;
      std::array<float, 16> position = modelview_stimulus.ToGlArray();
      glUniformMatrix4fv(0, 1, GL_FALSE, position.data());
      stimulus.Ready(lum, col);
      stimulus.Draw();
      CHECKGLERROR("DrawStimulus");
    }

    Shape OpiApp::getShapeCode(int type) {
      switch(type) { // defaults to circle
        case Shape::CIRCLE:  return circle;
        case Shape::SQUARE:  return square;
        case Shape::CROSS:   return cross;
        case Shape::MALTESE: return maltese;
        case Shape::ANNULUS: return annulus;
        default:             return nothing;
      }
    }
    float OpiApp::RadiansToDegrees(float angle) {
      return 180 / float(M_PI) * angle;
    }

    float OpiApp::DegreesToRadians(float angle) {
      return float(M_PI) / 180 * angle;
    }

    float OpiApp::degOfViewToLength(int distance, float angle) {
      return (float) distance * tan(angle);
    }

}  // namespace ndk_opi
