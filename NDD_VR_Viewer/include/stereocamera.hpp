#ifndef __MSI_VR__STEREOCAMERA_HPP__
#define __MSI_VR__STEREOCAMERA_HPP__

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"

namespace msi_vr
{
    class StereoCamera : public Camera
    {
    public:
        float ipd = 0.065f; //interpupillary distance

        explicit StereoCamera(float ipd = 0.065f, glm::vec3 const &position = glm::vec3{0.f, 0.f, 1.f}, glm::vec3 const &front = glm::vec3{0.f, 0.f, -1.f}, glm::vec3 const &up = glm::vec3{0.f, 1.f, 0.f}, float fovy = glm::radians(45.f), float aspectratio = 1.f, float minDist = 0.1f, float maxDist = 100.f)
            : Camera(position, front, up, fovy, aspectratio, minDist, maxDist), ipd(ipd)
        {
            leftCamera = std::make_unique<Camera>(-ipd * right, front, up, fovy, aspectratio, minDist, maxDist);
            rightCamera = std::make_unique<Camera>(+ipd * right, front, up, fovy, aspectratio, minDist, maxDist);
        }

        virtual ~StereoCamera()
        {
        }

        glm::mat4 getViewMatrix(bool left)
        {
            return (left ? leftCamera->viewmatrix : rightCamera->viewmatrix) * this->viewmatrix ;
        }

    private:
        std::unique_ptr<Camera> leftCamera, rightCamera;
    };

} // namespace msi_vr

#endif