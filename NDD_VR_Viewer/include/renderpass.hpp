#ifndef __MSI_VR__RENDERPASS_HPP__
#define __MSI_VR__RENDERPASS_HPP__

#include "texture.hpp"

namespace msi_vr
{
    class RenderPass
    {
    public:
    GLuint fbo;

    virtual ~RenderPass();

    virtual void use() const;

    static RenderPass& getScreenRenderPass() {return screenRenderPass;}
    protected:
    RenderPass(bool create = true);

    private:
    static RenderPass screenRenderPass;
    };

    class TextureRenderPass : public RenderPass
    {
        public:
        GLuint rbo;     // renderbuffer object for depth
        Texture target; // rendertarget texture

        TextureRenderPass();
        TextureRenderPass(int w, int h, GLint format = GL_RGBA);
        void createTexture(int w, int h, GLint format = GL_RGBA);
        virtual ~TextureRenderPass();
        private:
    };


} // namespace msi_vr

#endif