//
// Created by 厉猛 on 2024-06-07.
//

#ifndef GAMEPROGRAMMINGINCPP_SHADER_HPP
#define GAMEPROGRAMMINGINCPP_SHADER_HPP


#include <string>
#include <GL/glew.h>
#include "../math/Matrix.hpp"

class Shader {
public:
    Shader() = default;
    ~Shader() = default;

    /// 基于名字加载指定的顶点/片元着色器对
    bool load(const std::string &vertName, const std::string &fragName);
    /// 激活当前着色器程序
    void setActive();
    /// 卸载着色器程序并删除着色器对象
    void unload();

    void setFloatUniform(const char *uniformName, float val);
    void setMatrixUniform(const char *name, const Matrix4 &matrix);
    void setVectorUniform(const char *uniformName, Vector3 vector);

private:
    /// 尝试编译指定的着色器源码
    bool compile(const std::string &fileName, GLenum shaderType, GLuint &outShaderId);
    /// 判断指定的着色器是否已编译
    bool isCompiled(GLuint shaderId);

    /// 判断顶点/片元着色器对是否已链接
    bool isValidProgram();

    /// 顶点着色器的ID
    GLuint vertexShaderId;
    /// 片元着色器的ID
    GLuint fragShaderId;
    /// 着色器程序的ID
    GLuint shaderProgramId;
};


#endif //GAMEPROGRAMMINGINCPP_SHADER_HPP
