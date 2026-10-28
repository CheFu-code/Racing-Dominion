#include "Renderer.h"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOG_TAG "Renderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace RacingEngine {

// Embedded shaders
static const char* vertexShaderSource = R"(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

void main() {
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = mat3(uModelMatrix) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
}
)";

static const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uObjectColor;
uniform vec3 uAmbientColor;
uniform sampler2D uTexture;
uniform int uUseTexture;
uniform float uEmissive;
uniform float uOpacity;
uniform int uSoftParticle;

out vec4 FragColor;

void main() {
    vec3 baseColor = uObjectColor;
    if (uUseTexture == 1) {
        baseColor = texture(uTexture, vTexCoord).rgb;
    }

    vec3 ambient = uAmbientColor * baseColor;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 0.95) * baseColor;

    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * vec3(1.0);

    vec3 result = mix(ambient + diffuse + specular, baseColor * 1.2, uEmissive);
    float fog = smoothstep(60.0, 240.0, length(uViewPos - vFragPos));
    result = mix(result, vec3(0.14, 0.065, 0.185), fog * 0.85);
    float alpha = uOpacity;
    if (uSoftParticle == 1) alpha *= 1.0 - smoothstep(0.08, 0.5, length(vTexCoord - vec2(0.5)));
    FragColor = vec4(result, alpha);
}
)";

Renderer::Renderer()
    : lightPosition(0, 50, 50),
      ambientR(0.32f), ambientG(0.24f), ambientB(0.40f),
      screenWidth(0), screenHeight(0) {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize() {
    LOGI("Initializing renderer...");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    if (!loadShaders()) {
        LOGE("Failed to load shaders");
        return false;
    }

    LOGI("Renderer initialized");
    return true;
}

void Renderer::resize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)(height > 0 ? height : 1);
    camera.setPerspective(60.0f, aspect, 0.1f, 1000.0f);
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    Matrix4 view = camera.getViewMatrix();
    Matrix4 proj = camera.getProjectionMatrix();
    shader.setMat4("uViewMatrix", view.data());
    shader.setMat4("uProjectionMatrix", proj.data());

    shader.setVec3("uLightPos", lightPosition.x, lightPosition.y, lightPosition.z);
    Vector3 camPos = camera.getPosition();
    shader.setVec3("uViewPos", camPos.x, camPos.y, camPos.z);
    shader.setVec3("uAmbientColor", ambientR, ambientG, ambientB);
    shader.setInt("uUseTexture", 0);
    shader.setFloat("uEmissive", 0);
    shader.setFloat("uOpacity", 1);
    shader.setInt("uSoftParticle", 0);
}

void Renderer::renderModel(Model& model, const Vector3& color) {
    shader.setVec3("uObjectColor", color.x, color.y, color.z);
    model.draw(shader);
}

void Renderer::endFrame() {
    shader.unuse();
}

void Renderer::shutdown() {
    shader.~Shader();
    new (&shader) Shader();
}

void Renderer::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

bool Renderer::loadShaders() {
    return shader.loadFromString(vertexShaderSource, fragmentShaderSource);
}

Mesh* Renderer::createPlaneMesh(float width, float depth, float uvScale) {
    std::vector<Vertex> vertices(4);
    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    vertices[0] = {{-hw, 0, -hd}, {0, 1, 0}, {0, 0}};
    vertices[1] = {{ hw, 0, -hd}, {0, 1, 0}, {uvScale, 0}};
    vertices[2] = {{ hw, 0,  hd}, {0, 1, 0}, {uvScale, uvScale}};
    vertices[3] = {{-hw, 0,  hd}, {0, 1, 0}, {0, uvScale}};

    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

    Mesh* mesh = new Mesh();
    mesh->upload(vertices, indices);
    return mesh;
}

Mesh* Renderer::createCubeMesh(float size) {
    float s = size * 0.5f;
    std::vector<Vertex> vertices = {
        // Front
        {{-s, -s,  s}, {0, 0, 1}, {0, 0}},
        {{ s, -s,  s}, {0, 0, 1}, {1, 0}},
        {{ s,  s,  s}, {0, 0, 1}, {1, 1}},
        {{-s,  s,  s}, {0, 0, 1}, {0, 1}},
        // Back
        {{ s, -s, -s}, {0, 0, -1}, {0, 0}},
        {{-s, -s, -s}, {0, 0, -1}, {1, 0}},
        {{-s,  s, -s}, {0, 0, -1}, {1, 1}},
        {{ s,  s, -s}, {0, 0, -1}, {0, 1}},
        // Top
        {{-s,  s,  s}, {0, 1, 0}, {0, 0}},
        {{ s,  s,  s}, {0, 1, 0}, {1, 0}},
        {{ s,  s, -s}, {0, 1, 0}, {1, 1}},
        {{-s,  s, -s}, {0, 1, 0}, {0, 1}},
        // Bottom
        {{-s, -s, -s}, {0, -1, 0}, {0, 0}},
        {{ s, -s, -s}, {0, -1, 0}, {1, 0}},
        {{ s, -s,  s}, {0, -1, 0}, {1, 1}},
        {{-s, -s,  s}, {0, -1, 0}, {0, 1}},
        // Right
        {{ s, -s,  s}, {1, 0, 0}, {0, 0}},
        {{ s, -s, -s}, {1, 0, 0}, {1, 0}},
        {{ s,  s, -s}, {1, 0, 0}, {1, 1}},
        {{ s,  s,  s}, {1, 0, 0}, {0, 1}},
        // Left
        {{-s, -s, -s}, {-1, 0, 0}, {0, 0}},
        {{-s, -s,  s}, {-1, 0, 0}, {1, 0}},
        {{-s,  s,  s}, {-1, 0, 0}, {1, 1}},
        {{-s,  s, -s}, {-1, 0, 0}, {0, 1}},
    };

    std::vector<unsigned int> indices;
    for (int face = 0; face < 6; face++) {
        unsigned int base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh* mesh = new Mesh();
    mesh->upload(vertices, indices);
    return mesh;
}

} // namespace RacingEngine
