#ifndef __MSI_VR__ODSSPHEREOBJECT_HPP__
#define __MSI_VR__ODSSPHEREOBJECT_HPP__

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "globject.hpp"

namespace msi_vr
{
    class ODSSphereObject
    {
    public:
        enum Mode
        {
            MESH,
            POINTCLOUD
        };
        int width, height;
        float ipd;
        Mode mode;

        std::vector<float> verticesLeft;
        std::vector<float> verticesRight;
        std::vector<float> verticesSpherical;
        std::vector<float> verticesIndices;
        std::vector<int> dimensions;

        std::vector<unsigned int> indices;

        GLObject leftODS, rightODS;

        glm::mat4 modelmatrix;

        bool initialized = false;

        ODSSphereObject();
        ODSSphereObject(int width, int height, Mode mode = Mode::MESH, float ipd = 0.065f, float radius = 1.0f, glm::vec3 const &pos = {0.f, 0.f, 0.f});

        void initializeObject(int width, int height, Mode mode = Mode::MESH, float ipd = 0.065f, float radius = 1.0f, glm::vec3 const &pos = {0.f, 0.f, 0.f});
        void setVerticesPositions(float radius = 1.f);

        glm::vec3 getViewcirclePosition(float phi) const;
        glm::vec3 getViewcircleDirection(float phi, float theta) const;

        void draw(bool left) const;

        void changeOverlap(double change);
    private:
        float overlap = 0.f;
    };
} // namespace msi_vr

#endif