#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace msi_vr
{
    Texture::Texture(TextureOptions const &options)
        : mipmaps(options.mipmaps)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrap_t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.max_filter);

    }

    Texture::Texture(int w, int h, GLint format, TextureOptions const &options)
        : Texture(options) 
    {
        initialize(w, h, format);
    }

    Texture::Texture(std::string const &filename, GLint internalFormat, TextureOptions const &options)
        : Texture(options)
    {
        loadTexture(filename, internalFormat, options);
    }

    Texture::~Texture()
    {
    }

    bool Texture::loadTexture(std::string const &filename, GLint internalFormat, TextureOptions const &options)
    {
        int nrChannels;
        uint8_t *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
        GLint fileFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;
        initialize(width, height, internalFormat);
        if(data)
        {
          upload(data, fileFormat);
        }
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool Texture::initialize(int width, int height, GLint format, uint8_t *data, GLint dataFormat)
    {
        this->width = width;
        this->height = height;
        this->format = format;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        if(mipmaps) glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return initialized = true;
    }

    bool Texture::upload(uint8_t *data, GLint dataFormat)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
} // namespace msi_vr