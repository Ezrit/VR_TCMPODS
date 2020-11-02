#ifndef __MSI_VR__TEXTURE_HPP__
#define __MSI_VR__TEXTURE_HPP__

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace msi_vr
{
    class Texture
    {
        public:
        struct TextureOptions
        {
            GLint wrap_s;
            GLint wrap_t;
            GLint min_filter;
            GLint max_filter;

            bool mipmaps;
        };
        int width, height;
        bool initialized = false;

        GLuint texture;
        GLint format;
        bool mipmaps;

        Texture(TextureOptions const &options = {GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, true});
        Texture(int w, int h, GLint format = GL_RGBA, TextureOptions const &options = {GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, true});
        Texture(std::string const &filename, GLint internalFormat = GL_RGBA, TextureOptions const &options = {GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, true});
        ~Texture();

        bool loadTexture(std::string const &filename, GLint internalFormat = GL_RGBA, TextureOptions const &options = {GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, true});
        bool initialize(int width, int height, GLint format = GL_RGBA, uint8_t *data = NULL, GLint dataFormat = GL_RGBA);
        bool upload(uint8_t *data, GLint dataFormat = GL_RGBA);
        private:
    };
} // namespace msi_vr

#endif