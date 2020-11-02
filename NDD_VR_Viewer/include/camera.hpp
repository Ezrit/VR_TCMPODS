#ifndef __MSI_VR__CAMERA_HPP__
#define __MSI_VR__CAMERA_HPP__

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace msi_vr
{

    class Camera
    {
        public:
        glm::vec4 position;
        glm::vec4 front;
        glm::vec4 right;
        glm::vec4 up;
        glm::mat4 viewmatrix = glm::mat4(1.f);

        float speed = 1.00f;
        glm::vec3 moveDirection;
        float sensitivity = 0.1f;

        float fovy;
        float aspectratio;
        float minDist, maxDist;
        glm::mat4 projectionmatrix;

        float yawRotation = 0.f, pitchRotation = 0.f;

        explicit Camera(glm::vec3 const &position, glm::vec3 const &front, glm::vec3 const &up, float fovy = glm::radians(45.f), float aspectratio = 1.f, float minDist = 0.1f, float maxDist = 100.f)
            : position({position, 1.0}), front({glm::normalize(front), 0.0}), up({glm::normalize(up), 0.0f}), right({glm::normalize(glm::cross(front, up)), 0.0}), fovy(fovy), aspectratio(aspectratio), minDist(minDist), maxDist(maxDist), moveDirection(glm::vec3{0.f,0.f,0.f})
        {
            viewmatrix = glm::lookAt(position, position+front, up);
            projectionmatrix = glm::perspective(fovy, aspectratio, minDist, maxDist);
        }

        virtual ~Camera() {}

        virtual void move(glm::vec3 dir)
        {
            moveDirection += dir;
        }

        virtual void rotate(float yaw, float pitch)
        {
            pitchRotation = std::min(90.0f, std::max(-90.0f, pitchRotation + pitch));
            yawRotation = fmod(yawRotation + yaw, 360.0f);
        }

        virtual void update(float deltaTime)
        {
            if(glm::length(moveDirection) > 0) moveDirection = glm::normalize(moveDirection);

            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(pitchRotation), glm::vec3(right));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(yawRotation), glm::vec3(up));

            position += speed * deltaTime * glm::transpose(rotationMatrix) * glm::vec4(moveDirection, 0);

            glm::vec4 frontRotated = glm::normalize(glm::transpose(rotationMatrix) * front);
            glm::vec4 upRotated = glm::normalize(glm::transpose(rotationMatrix) * up);
            glm::vec4 rightRotated = glm::normalize(glm::transpose(rotationMatrix) * right);

            viewmatrix = glm::lookAt(glm::vec3{position}, glm::vec3{position}+glm::vec3{frontRotated}, glm::vec3{upRotated});

            moveDirection = glm::vec3{0.f};
        }
    };

} // namespace msi_vr

#endif