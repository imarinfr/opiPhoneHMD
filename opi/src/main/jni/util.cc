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

#include "util.h"

#include <android/log.h>
#include <unistd.h>

#include <array>
#include <cmath>
#include <random>
#include <sstream>
#include <string>

#include <GLES3/gl32.h>

namespace ndk_opi {

    void None(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        // vertices
        addVertex(0, 0, 0, v);
        addVertex(0, 0, 0, v);
        addVertex(0, 0, 0, v);
        // indices
        ind->push_back(0);
        ind->push_back(1);
        ind->push_back(2);
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void Circle(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        int n = 500;
        // vertices and indices
        addVertex(0, 0, 0, v);
        ind->push_back(0);
        for (int i = 0; i < n + 1; i++) {
            float theta = 2.0f * float(M_PI) * float(i) / float(n);
            addVertex(cosf(theta), sinf(theta), 0, v);
            ind->push_back(i + 1);
        }
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void Square(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        // vertices
        addVertex(1, 1, 0, v);
        addVertex(-1, 1, 0, v);
        addVertex(-1, -1, 0, v);
        addVertex(1, -1, 0, v);
        // indices
        ind->push_back(0);
        ind->push_back(1);
        ind->push_back(2);
        ind->push_back(2);
        ind->push_back(3);
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void Cross(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        // vertices
        addVertex(1.00, 0.10, 0, v);
        addVertex(-1.00, 0.10, 0, v);
        addVertex(-1.00, -0.10, 0, v);
        addVertex(1.00, -0.10, 0, v);
        addVertex(-0.10, -1.00, 0, v);
        addVertex(0.10, -1.00, 0, v);
        addVertex(0.10, 1.00, 0, v);
        addVertex(-0.10, 1.00, 0, v);
        // indices
        for (int i = 0; i < 2; i++) {
            ind->push_back(4 * i);
            ind->push_back(4 * i + 1);
            ind->push_back(4 * i + 2);
            ind->push_back(4 * i);
            ind->push_back(4 * i + 2);
            ind->push_back(4 * i + 3);
        }
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void
    MalteseCross(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        // vertices
        addVertex(1.00, 0.20, 0, v);
        addVertex(0.00, 0.02, 0, v);
        addVertex(0.00, -0.02, 0, v);
        addVertex(1.00, -0.20, 0, v);
        addVertex(-0.02, 0.00, 0, v);
        addVertex(0.02, 0.00, 0, v);
        addVertex(0.20, 1.00, 0, v);
        addVertex(-0.20, 1.00, 0, v);
        addVertex(0.20, -1.00, 0, v);
        addVertex(0.02, 0.00, 0, v);
        addVertex(-0.02, 0.00, 0, v);
        addVertex(-0.20, -1.00, 0, v);
        addVertex(-1.00, 0.20, 0, v);
        addVertex(-1.00, -0.20, 0, v);
        addVertex(0.00, -0.02, 0, v);
        addVertex(0.00, 0.02, 0, v);
        // indices
        for (int i = 0; i < 8; i++) {
            ind->push_back(4 * i);
            ind->push_back(4 * i + 1);
            ind->push_back(4 * i + 2);
            ind->push_back(4 * i);
            ind->push_back(4 * i + 2);
            ind->push_back(4 * i + 3);
        }
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void Annulus(std::vector<GLfloat> *v, std::vector<GLfloat> *vt, std::vector<GLushort> *ind) {
        int n = 500;
        float r = 0.6; // ratio between outer and inner circle
        // vertices and indices
        for (int i = 0; i < n + 1; i++) {
            float theta1 = 2.0f * float(M_PI) * float(i) / float(n);
            float theta2 = 2.0f * float(M_PI) * float(i + 0.5) / float(n);
            addVertex(r * cosf(theta1), r * sinf(theta1), 0, v);
            addVertex(cosf(theta2), sinf(theta2), 0, v);
            ind->push_back(2 * i);
            ind->push_back(2 * i + 1);
        }
        // UV
        addUv(0, 0, vt); // UV attribute not used at the moment
    }

    void addVertex(float v1, float v2, float v3, std::vector<GLfloat> *v) {
        v->push_back(v1);
        v->push_back(v2);
        v->push_back(v3);
    }

    void addUv(float v1, float v2, std::vector<GLfloat> *vt) {
        vt->push_back(v1);
        vt->push_back(v2);
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &right) {
        Matrix4x4 result = right;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += this->m[k][j] * right.m[i][k];
                }
            }
        }
        return result;
    }

    std::array<float, 4> Matrix4x4::operator*(const std::array<float, 4> &vec) {
        std::array<float, 4> result = vec;
        for (int i = 0; i < 4; ++i) {
            result[i] = 0;
            for (int k = 0; k < 4; ++k) {
                result[i] += this->m[k][i] * vec[k];
            }
        }
        return result;
    }

    std::array<float, 16> Matrix4x4::ToGlArray() {
        std::array<float, 16> result = {};
        memcpy(&result[0], m, 16 * sizeof(float));
        return result;
    }

    Matrix4x4 GetMatrixFromGlArray(float *vec) {
        Matrix4x4 result = {};
        memcpy(result.m, vec, 16 * sizeof(float));
        return result;
    }

    Matrix4x4
    GetAffineMatrix(float sx, float sy, float theta, const std::array<float, 3> &translation) {
        return {{{sx * cos(theta), -sx * sin(theta), 0.0f, 0.0f},
                     {sy * sin(theta), sy * cos(theta), 0.0f, 0.0f},
                     {0.0f, 0.0f, 1.0f, 0.0f},
                     {translation.at(0), translation.at(1), -translation.at(2), 1.0f}}};
    }

    void CheckGlError(const char *file, int line, const char *label) {
        uint gl_error = glGetError();
        if(gl_error != GL_NO_ERROR) {
            LOGE("%s : %d > GL error @ %s: %d", file, line, label, gl_error);
            // Crash immediately to make OpenGL errors obvious.
            abort();
        }
    }

    GLuint LoadGLShader(GLenum type, const char *shader_source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shader_source, nullptr);
        glCompileShader(shader);

        // Get the compilation status.
        GLint compile_status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

        // If the compilation failed, delete the shader and show an error.
        if (compile_status == 0) {
            GLint info_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
            if (info_len == 0) {
                return 0;
            }
            std::vector<char> info_string(info_len);
            glGetShaderInfoLog(shader, (GLint) info_string.size(), nullptr, info_string.data());
            LOGE("Could not compile shader of type %d: %s", type, info_string.data());
            glDeleteShader(shader);
            return 0;
        } else {
            return shader;
        }
    }

    void Shape::Initialize(GLuint pos_a, GLuint uv_a, int type) {
        position_attrib = pos_a;
        uv_attrib = uv_a;
        switch(type) { // defaults to circle
            case CIRCLE:
                native = GL_TRIANGLE_FAN;
                Circle(&vertices, &uv, &indices);
                break;
            case SQUARE:
                native = GL_TRIANGLE_FAN;
                Square(&vertices, &uv, &indices);
                break;
            case CROSS:
                native = GL_TRIANGLES;
                Cross(&vertices, &uv, &indices);
                break;
            case MALTESE:
                native = GL_TRIANGLES;
                MalteseCross(&vertices, &uv, &indices);
                break;
            case ANNULUS:
                native = GL_TRIANGLE_STRIP;
                Annulus(&vertices, &uv, &indices);
                break;
            default:
                native = GL_TRIANGLES;
                None(&vertices, &uv, &indices);
        }
        glGenTextures(1, &texture_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Shape::Ready(float luminance, const float color[]) const {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        auto *texData = new unsigned char[3 * sizeof(unsigned char)];
        texData[0] = (unsigned char) (255.0f * luminance * color[0]);
        texData[1] = (unsigned char) (255.0f * luminance * color[1]);
        texData[2] = (unsigned char) (255.0f * luminance * color[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        delete[] texData;
    }

    void Shape::Draw() const {
        glEnableVertexAttribArray(position_attrib);
        glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, vertices.data());
        glEnableVertexAttribArray(uv_attrib);
        glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, false, 0, uv.data());
        glDrawElements(native, (GLint) indices.size(), GL_UNSIGNED_SHORT, indices.data());
    }

    Shape::~Shape() {
        if (texture_id != 0) {
            glDeleteTextures(1, &texture_id);
        }
    }  // namespace ndk_opi
}