//
// Created by 厉猛 on 2024-06-07.
//

#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <SDL_log.h>

bool Shader::load(const std::string &vertName, const std::string &fragName) {
    // 编译着色器对象
    if (!compile(vertName, GL_VERTEX_SHADER, vertexShaderId)
        || !compile(fragName, GL_FRAGMENT_SHADER, fragShaderId)) {
        return false;
    }
    // 将顶点着色器和片元着色器链接起来创建着色器程序对象
    shaderProgramId = glCreateProgram();
    glAttachShader(shaderProgramId, vertexShaderId);
    glAttachShader(shaderProgramId, fragShaderId);
    glLinkProgram(shaderProgramId);

    return isValidProgram();
}

bool Shader::compile(const std::string &fileName, GLenum shaderType, GLuint &outShaderId) {
    std::ifstream shaderFile(fileName);
    if (shaderFile.is_open()) {
        std::stringstream ss;
        ss << shaderFile.rdbuf();
        std::string source = ss.str();
        const auto sourceAsCStr = source.c_str();

        // 创建着色器对象，返回ID并编译着色器
        outShaderId = glCreateShader(shaderType);
        glShaderSource(outShaderId, 1, &sourceAsCStr, nullptr);
        glCompileShader(outShaderId);

        if (!isCompiled(outShaderId)) {
            SDL_Log("Failed to compile shader %s", fileName.c_str());
            return false;
        }
    } else {
        SDL_Log("Shader file not found: %s", fileName.c_str());
        return false;
    }
    return true;
}

void Shader::unload() {
    glDeleteProgram(shaderProgramId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragShaderId);
}

void Shader::setActive() {
    glUseProgram(shaderProgramId);
}

bool Shader::isCompiled(GLuint shaderId) {
    GLint status;
    // 查询指定着色器的编译状态
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetShaderInfoLog(shaderId, 511, nullptr, buffer);
        SDL_Log("GLSL Compile Failed:\n%s", buffer);
        return false;
    }
    return true;
}

bool Shader::isValidProgram() {
    GLint status;
    // 查询指定着色器的编译状态
    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetProgramInfoLog(shaderProgramId, 511, nullptr, buffer);
        SDL_Log("Shader Program Link Failed:\n%s", buffer);
        return false;
    }
    return true;
}

void Shader::setMatrixUniform(const char *name, const Matrix4 &matrix) {
    // 获取指定 uniform 参数名的位置
    // TODO 可缓存记录，不必每次都查询位置
    GLuint loc = glGetUniformLocation(shaderProgramId, name);
    // 指定矩阵类型的 uniform 参数
    // 参数：uniform位置，矩阵的数量，使用行向量，将矩阵对象转化为浮点数组以传递
    glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.getAsFloatPtr());
}

void Shader::setFloatUniform(const char *uniformName, float val) {
    // 获取指定 uniform 参数名的位置
    // TODO 可缓存记录，不必每次都查询位置
    GLuint loc = glGetUniformLocation(shaderProgramId, uniformName);
    // 指定矩阵类型的 uniform 参数
    // 参数：uniform位置，矩阵的数量，使用行向量，将矩阵对象转化为浮点数组以传递
    glUniform1f(loc, val);
}
