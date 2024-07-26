//
// Created by 厉猛 on 2024-06-07.
//

#ifndef GAMEPROGRAMMINGINCPP_VERTEXARRAY_HPP
#define GAMEPROGRAMMINGINCPP_VERTEXARRAY_HPP


class VertexArray {
public:
    VertexArray(const float *verts, unsigned int vnum, const unsigned int *indices, unsigned int inum);
    ~VertexArray();

    /// 将当前顶点数组对象设为活跃）
    void setActive();

    unsigned int getVertexCount() const { return vertexCount; }
    unsigned int getIndexCount() const { return indexCount; }
private:
    /// 顶点缓冲区的元素数
    unsigned int vertexCount;
    /// 索引缓冲区的元素数
    unsigned int indexCount;
    /// 顶点缓冲区的ID
    unsigned int vertexBufferId;
    /// 索引缓冲区的ID
    unsigned int indexBufferId;
    /// VAO的ID
    unsigned int vertexArray;
};


#endif //GAMEPROGRAMMINGINCPP_VERTEXARRAY_HPP
