#ifndef SHADER_H
#define SHADER_H

#include <GLES3/gl3.h>
#include <string>

namespace RacingEngine {

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromString(const char* vertexSource, const char* fragmentSource);
    bool loadFromFile(const char* vertexPath, const char* fragmentPath);
    
    void use();
    void unuse();
    
    // Uniform setters
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setMat4(const char* name, const float* matrix) const;
    
    GLuint getProgramId() const { return programId; }

private:
    GLuint programId;
    
    GLuint compileShader(GLenum type, const char* source);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
};

} // namespace RacingEngine

#endif // SHADER_H
