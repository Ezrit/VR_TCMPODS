#ifndef __MSI_VR__VRCONTEXT_HPP__
#define __MSI_VR__VRCONTEXT_HPP__

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "openvr.h"

namespace msi_vr
{
    class VRContext
    {
    public:
        float minDist, maxDist;
        vr::IVRSystem *hmd;
        vr::IVRCompositor *compositor;
        glm::mat4 viewmatrixHmd = glm::mat4(1.f); // world -> hmd
        glm::mat4 viewmatrixLeft = glm::mat4(1.f); // hmd -> left eye
        glm::mat4 viewmatrixRight = glm::mat4(1.f); // hmd -> right eye

        glm::mat4 projectionmatrixLeft;
        glm::mat4 projectionmatrixRight;

        bool initialized = false;

        vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];

        uint32_t width, height;

        explicit VRContext(float frustumNear = .1f, float frustumFar = 100.f);

        virtual void update();
        virtual void render(GLint textureLeft, GLint textureRight);

        glm::mat4 getViewMatrix(bool left) const;
        glm::mat4 getProjectionMatrix(bool left) const;

    private:
        bool initialize();

    };
} // namespace msi_vr

#endif