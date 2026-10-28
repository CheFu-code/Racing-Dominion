#include "Shader.h"
#include <android/log.h>
#include <android/asset_manager.h>

#define LOG_TAG "Shader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace RacingEngine {

Shader::Shader() : programId(0) {}

Shader::~Shader() {
    if (programId != 0) {
        glDeleteProgram(programId);
    }
}

bool Shader::loadFromString(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) return false;

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    bool success = linkProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success;
}

bool Shader::loadFromFile(const char* vertexPath, const char* fragmentPath) {
    // This method requires an AAssetManager to be available
    // In practice, shaders should be loaded via loadFromString with embedded sources
    // or by using AssetLoader to read files
    LOGE("loadFromFile requires external asset management - use loadFromString instead");
    return false;
}

void Shader::use() {
    glUseProgram(programId);
}

void Shader::unuse() {
    glUseProgram(0);
}

void Shader::setInt(const char* name, int value) const {
    GLint location = glGetUniformLocation(programId, name);
    if (location >= 0) glUniform1i(location, value);
}

void Shader::setFloat(const char* name, float value) const {
    GLint location = glGetUniformLocation(programId, name);
    if (location >= 0) glUniform1f(location, value);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    GLint location = glGetUniformLocation(programId, name);
    if (location >= 0) glUniform3f(location, x, y, z);
}

void Shader::setMat4(const char* name, const float* matrix) const {
    GLint location = glGetUniformLocation(programId, name);
    if (location >= 0) glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("Shader compilation failed: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        LOGE("Program linking failed: %s", infoLog);
        return false;
    }

    return true;
}

} // namespace RacingEngine
