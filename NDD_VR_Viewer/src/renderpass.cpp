#include "renderpass.hpp"

namespace msi_vr
{
    RenderPass RenderPass::screenRenderPass{false};

    RenderPass::RenderPass(bool create)
    {
        if(create) glCreateFramebuffers(1, &fbo);
        else fbo = 0;
    }

    RenderPass::~RenderPass()
    {
        if(fbo != 0) glDeleteFramebuffers(1, &fbo);
    }

    void RenderPass::use() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    TextureRenderPass::TextureRenderPass()
        : RenderPass(), target(Texture::TextureOptions{GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, false})
    {
    }


    TextureRenderPass::TextureRenderPass(int w, int h, GLint format)
        : RenderPass(), target(Texture::TextureOptions{GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR, false})
    {
        createTexture(h, w, format);
    }

    TextureRenderPass::~TextureRenderPass()
    {
        if(rbo != 0) glDeleteRenderbuffers(1, &rbo);
    }

    void TextureRenderPass::createTexture(int w, int h, GLint format)
    {
        if(rbo != 0) glDeleteRenderbuffers(1, &rbo);

        target.initialize(w, h, format);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // attach the texture to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);

        // create a depth buffer
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach the depth buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} // namespace msi_vr
