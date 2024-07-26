//
// Created by 厉猛 on 2024-06-07.
//

#include <GL/glew.h>
#include "VertexArray.hpp"

VertexArray::VertexArray(const float *verts, unsigned int vnum, const unsigned int *indices, unsigned int inum) {
    // 创建VAO并绑定
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    // 创建顶点缓冲区并绑定
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    // 传递顶端缓冲区的数据
    // 参数设置：类型为数组缓冲，要缓冲的字节数，数据起始点（指针），数据的使用方式（载入一次，作为绘制频繁使用）
    glBufferData(GL_ARRAY_BUFFER, vnum * 5 * sizeof(float), verts, GL_STATIC_DRAW);

    // 创建下标缓冲区并绑定
    glGenBuffers(1, &indexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inum * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // 启用第一个顶点属性数组（顶点坐标）
    glEnableVertexAttribArray(0);
    // 设置顶点属性信息
    // 参数：第几个顶点属性数组，每个组件的大小，组件的类型，是否为整型类型，步进量，偏移量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5, 0);
    // 启用第二个顶点属性数组（UV坐标）
    glEnableVertexAttribArray(1);
    // 设置顶点属性信息
    // 参数：第几个顶点属性数组，每个组件的大小，组件的类型，是否为整型类型，步进量，偏移量
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5, reinterpret_cast<void *>(sizeof(float) * 3));
}

VertexArray::~VertexArray() {
    glDeleteBuffers(1, &vertexBufferId);
    glDeleteBuffers(1, &indexBufferId);
    glDeleteVertexArrays(1, &vertexArray);
}

void VertexArray::setActive() {
    glBindVertexArray(vertexArray);
}
