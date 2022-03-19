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

#ifndef OPI_ANDROID_SRC_MAIN_JNI_UTIL_H_
#define OPI_ANDROID_SRC_MAIN_JNI_UTIL_H_

#include <jni.h>

#include <array>
#include <vector>

#include <GLES2/gl2.h>

#include <android/asset_manager.h>

#define LOG_TAG "OpiApp"
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace ndk_opi {

    class Matrix4x4 {
    public:
        float m[4][4];

        // Multiplies two matrices.
        Matrix4x4 operator*(const Matrix4x4& right);

        // Multiplies a matrix with a vector.
        std::array<float, 4> operator*(const std::array<float, 4>& vec);

        // Converts a matrix to an array of floats suitable for passing to OpenGL.
        std::array<float, 16> ToGlArray();
    };

/**
 * Converts an array of floats to a matrix.
 *
 * @param vec GL array
 * @return Obtained matrix
 */
    Matrix4x4 GetMatrixFromGlArray(float* vec);

/**
 * Construct a translation matrix.
 *
 * @param translation Translation array
 * @return Obtained matrix
 */
    Matrix4x4 GetAffineMatrix(float sx, float sy, float theta, const std::array<float, 3>& translation);

/**
 * Checks for OpenGL errors, and crashes if one has occurred.  Note that this
 * can be an expensive call, so real applications should call this rarely.
 *
 * @param file File name
 * @param line Line number
 * @param label Error label
 */
    void CheckGlError(const char* file, int line, const char* label);

    void None(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void Circle(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void Square(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void Cross(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void MalteseCross(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void Annulus(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind);

    void addVertex(float v1, float v2, float v3, std::vector<GLfloat> *v);

    void addUv(float v1, float v2, std::vector<GLfloat> *vt);

#define CHECKGLERROR(label) CheckGlError(__FILE__, __LINE__, label)

/**
 * Converts a string into an OpenGL ES shader.
 *
 * @param type The type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
 * @param shader_source The source code of the shader.
 * @return The shader object handler, or 0 if there's an error.
 */
    GLuint LoadGLShader(GLenum type, const char* shader_source);

    class Shape {
    public:
        const static int NONE    = -1;
        const static int CIRCLE  = 0;
        const static int SQUARE  = 1;
        const static int CROSS   = 2;
        const static int MALTESE = 3;
        const static int ANNULUS = 4;

        Shape() = default;

        ~Shape();

        // Initializes the mesh from a .obj file.
        //
        // @return True if initialization was successful.
        void Initialize(GLuint pos_a, GLuint uv_a, int type);

        // Binds the texture, replacing any previously bound texture.
        // and sets up luminance and color
        void Ready(float luminance, const float color[]) const;

        // Draws the mesh. The u_MVP uniform should be set before calling this using
        // glUniformMatrix4fv(), and a texture should be bound to GL_TEXTURE0.
        void Draw() const;

    private:
        int native;
        std::vector<GLfloat> vertices;
        std::vector<GLfloat> uv;
        std::vector<GLushort> indices;
        GLuint position_attrib{0};
        GLuint uv_attrib{0};
        GLuint texture_id{0};
    };
}  // namespace ndk_opi

#endif  // OPI_ANDROID_SRC_MAIN_JNI_UTIL_H_
