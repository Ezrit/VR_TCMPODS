#ifndef __MSI_VR__GLOBJECT_HPP__
#define __MSI_VR__GLOBJECT_HPP__

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "drawable.hpp"

namespace msi_vr
{
    class GLObject : public Drawable
    {
        public:
        GLObject();
        virtual ~GLObject();
        
        virtual void draw() const override;

        template<typename T>
        int addVbo(std::vector<T> const &v, GLint size = 3, GLenum type = GL_FLOAT, GLboolean normalized = GL_FALSE, bool staticDraw=true)
        {
            GLuint vbo;
            glGenBuffers(1, &vbo);

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T) * v.size(), &v[0], staticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            glVertexAttribPointer(vbos.size(), size, type, normalized, size * sizeof(T), (void *)0);
            glEnableVertexAttribArray(vbos.size());
            glBindVertexArray(0);

            vbos.push_back(vbo);

            return vbos.size()-1;
        }

        template<typename T>
        void updateVBO(int index, std::vector<T> const &v, int size, int offset=0)
        {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
            glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), size * sizeof(T), &v[0]);
            glBindVertexArray(0);
        }

        void setEbo(std::vector<unsigned int> const &i);

        protected:
        GLuint vao = 0;
        std::vector<GLuint> vbos;
        GLuint ebo = 0;
        GLuint triangleCount = 0;
        private:
    };
}

#endif