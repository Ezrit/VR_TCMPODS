#include "vrcontext.hpp"

#include <iostream>

glm::mat4 ConvertOpenVRMatrixToGlm(const vr::HmdMatrix34_t &matPose) {

    return glm::mat4(
            matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
            matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
            matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
            matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
    );
}

glm::mat4 ConvertOpenVRMatrixToGlm(const vr::HmdMatrix44_t &matPose) {

    return glm::mat4(
            matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], matPose.m[3][0],
            matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], matPose.m[3][1],
            matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], matPose.m[3][2],
            matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], matPose.m[3][3]
    );
}

namespace msi_vr
{
    VRContext::VRContext(float frustumNear, float frustumFar)
        : minDist(frustumNear), maxDist(frustumFar)
    {
        initialize();

        width = height = 1024;
        if(!initialized) return;

        hmd->GetRecommendedRenderTargetSize(&width, &height);

        viewmatrixLeft = glm::inverse(ConvertOpenVRMatrixToGlm(hmd->GetEyeToHeadTransform(vr::Eye_Left)));
        viewmatrixRight = glm::inverse(ConvertOpenVRMatrixToGlm(hmd->GetEyeToHeadTransform(vr::Eye_Right)));
        projectionmatrixLeft = ConvertOpenVRMatrixToGlm(hmd->GetProjectionMatrix(vr::Eye_Left, minDist, maxDist));
        projectionmatrixRight = ConvertOpenVRMatrixToGlm(hmd->GetProjectionMatrix(vr::Eye_Right, minDist, maxDist));

    }

    void VRContext::update()
    {
        if(!initialized) return;
        compositor->WaitGetPoses(trackedDevicePoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

        for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
        {
            glm::mat4 pose = ConvertOpenVRMatrixToGlm(trackedDevicePoses[nDevice].mDeviceToAbsoluteTracking);

            if (trackedDevicePoses[nDevice].bPoseIsValid)
            {
                switch (hmd->GetTrackedDeviceClass(nDevice))
                {
                case vr::TrackedDeviceClass_HMD:
                    viewmatrixHmd = glm::inverse(pose);
                    break;
                default:
                    break;
                }
            }
        }
    }

    void VRContext::render(GLint textureLeft, GLint textureRight)
    {
        if(!initialized) return;
        vr::Texture_t left{(void*)textureLeft, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        vr::Texture_t right{(void*)textureRight, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        compositor->Submit(vr::Eye_Left, &left);
        compositor->Submit(vr::Eye_Right, &right);
    }

    bool VRContext::initialize()
    {
        vr::EVRInitError eError = vr::VRInitError_None;
        hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

        if (eError != vr::VRInitError_None)
        {
            hmd = NULL;
            std::cerr << "OpenVR : Unable to init VR runtime: ";
            std::cerr << vr::VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;

            return false;
        }

        compositor = vr::VRCompositor();
        if(!compositor)
        {
            std::cerr << "OpenVR : Unable to init Compositor" << std::endl;
            return false;
        }

        initialized = true;
    }

    glm::mat4 VRContext::getViewMatrix(bool left) const
    {
        return (left ? viewmatrixLeft : viewmatrixRight) * viewmatrixHmd;
    }

    glm::mat4 VRContext::getProjectionMatrix(bool left) const
    {
        return left ? projectionmatrixLeft : projectionmatrixRight;
    }
} // namespace msi_vr