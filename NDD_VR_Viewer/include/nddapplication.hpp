#ifndef __MSI_VR__NDDAPPLICATION_HPP__
#define __MSI_VR__NDDAPPLICATION_HPP__

#include "application.hpp"
#include "shader.hpp"
#include "stereocamera.hpp"
#include "vrcontext.hpp"

#include "videosynchronizer.hpp"
#include "odssphereobject.hpp"
#include "renderpass.hpp"
#include "quad.hpp"

namespace msi_vr
{
    class NDDApplication : public Application 
    {
        public:
        NDDApplication(GLFWwindow* window=initWindow(2048, 1024, "NDDApplication"));
        virtual ~NDDApplication();

        virtual void loadScene(std::string const &sceneName);


        virtual void update(std::chrono::duration<float> const &time);
        virtual void fixedUpdate(std::chrono::duration<float> const &time);
        virtual void render() const;

        protected:
        bool sceneInitialized = false;
        std::string sceneName;
        Shader foregroundNddShader;
        Shader nddShader;

        StereoCamera cam;
        VRContext vrContext;

        bool withVR = false;

        ODSSphereObject foreground, background, inpainting;

        VideoTexture foregroundMegaTexture;
        Texture backgroundMegaTexture;
        Texture inpaintingMegaTexture;

        RenderPass screenRenderPass = RenderPass::getScreenRenderPass();
        TextureRenderPass offscreenRenderPassLeft;
        TextureRenderPass offscreenRenderPassRight;
        Shader screenQuadShader{"shader/screenquad.vs", "shader/screenquad.fs"};
        Quad screenQuad{{-1.f,  1.f, 0.f, 
                      1.f,  1.f, 0.f, 
                     -1.f, -1.f, 0.f, 
                      1.f, -1.f, 0.f}};
    };
}

#endif