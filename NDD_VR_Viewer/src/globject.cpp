#include "globject.hpp"

namespace msi_vr
{
    GLObject::GLObject()
    {
        glGenVertexArrays(1, &vao);
    }

    GLObject::~GLObject()
    {
        if(vao != 0) glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(vbos.size(), &vbos[0]);
        if(ebo != 0) glDeleteBuffers(1, &ebo);
    }

    void GLObject::draw() const 
    {
        glBindVertexArray(vao);

        if(ebo == 0)
        {
            glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
        }
        else
        {
            glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_INT, 0);
        }
        

        glBindVertexArray(0);
    }


    void GLObject::setEbo(std::vector<unsigned int> const &i)
    {
        if (ebo == 0)
        {
            glGenBuffers(1, &ebo);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * i.size(), &i[0], GL_STATIC_DRAW);
        glBindVertexArray(0);

        triangleCount = i.size() / 3;
    }
} // namespace msi_vr