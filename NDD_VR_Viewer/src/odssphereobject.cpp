#include "odssphereobject.hpp"

#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

namespace msi_vr
{
    ODSSphereObject::ODSSphereObject()
    {
    }

    ODSSphereObject::ODSSphereObject(int width, int height, Mode mode, float ipd, float radius, glm::vec3 const &pos)
    {
        initializeObject(width, height, mode, ipd, radius, pos);
    }

    void ODSSphereObject::draw(bool left) const
    {
        if(!initialized) return;

        if(left) leftODS.draw();
        else rightODS.draw();
    }

    void ODSSphereObject::initializeObject(int width, int height, Mode mode, float ipd, float radius, glm::vec3 const &pos)
    {
        if(initialized) return;

        this->width = width;
        this->height = height;
        this->mode = mode;
        this->modelmatrix = glm::translate(glm::mat4(1.0f), pos);
        this->ipd = ipd;
        switch(mode)
        {
        case Mode::MESH:
            // need to have an extra column to let the shader correctly interpolate angles
            verticesLeft.resize(3 * (width + 1) * (height + 1));
            verticesRight.resize(3 * (width + 1) * (height + 1));
            verticesSpherical.resize(2 * (width + 1) * (height + 1));
            verticesIndices.resize(2 * (width + 1) * (height + 1));
            indices.resize(6 * width * height);

            for (int x = 0; x <= width; x++)
            {
                float phi = static_cast<float>(x) / static_cast<float>(width) * 2 * M_PI;

                glm::vec3 camerapos = ipd * glm::vec3{cos(phi), 0, sin(phi)};
                for (int y = 0; y <= height; y++)
                {
                    float theta = static_cast<float>(y) / static_cast<float>(height) * M_PI;
                    glm::vec3 cameradir{-sin(phi) * sin(theta), cos(theta), cos(phi) * sin(theta)};

                    verticesSpherical[x * (height + 1) * 2 + y * 2 + 0] = phi;
                    verticesSpherical[x * (height + 1) * 2 + y * 2 + 1] = theta;
                    verticesIndices[x * (height + 1) * 2 + y * 2 + 0] = x;
                    verticesIndices[x * (height + 1) * 2 + y * 2 + 1] = y;

                    glm::vec3 vertexLeft = camerapos + radius * cameradir;
                    verticesLeft[x * (height + 1) * 3 + y * 3 + 0] = vertexLeft.x;
                    verticesLeft[x * (height + 1) * 3 + y * 3 + 1] = vertexLeft.y;
                    verticesLeft[x * (height + 1) * 3 + y * 3 + 2] = vertexLeft.z;

                    glm::vec3 vertexRight = -camerapos + radius * cameradir;
                    verticesRight[x * (height + 1) * 3 + y * 3 + 0] = vertexRight.x;
                    verticesRight[x * (height + 1) * 3 + y * 3 + 1] = vertexRight.y;
                    verticesRight[x * (height + 1) * 3 + y * 3 + 2] = vertexRight.z;

                    if (x == width || y == height)
                        continue;

                    indices[x * height * 6 + y * 6 + 0] = (x * (height + 1) + y);
                    indices[x * height * 6 + y * 6 + 1] = (x * (height + 1) + y + 1);
                    indices[x * height * 6 + y * 6 + 2] = ((x + 1) * (height + 1) + y);
                    indices[x * height * 6 + y * 6 + 3] = (x * (height + 1) + y + 1);
                    indices[x * height * 6 + y * 6 + 4] = ((x + 1) * (height + 1) + y + 1);
                    indices[x * height * 6 + y * 6 + 5] = ((x + 1) * (height + 1) + y);
                }
            }
            break;
        case Mode::POINTCLOUD:
            verticesLeft.resize(4 * 3 * width * height);
            verticesRight.resize(4 * 3 * width * height);
            verticesSpherical.resize(2 * 4 * width * height);
            verticesIndices.resize(2 * 4 * width * height);
            indices.resize(6 * width * height);

            auto getVertexPos = [this](int x, int y) -> int
            {
                if(y < 0 || y >= this->height) return -1;
                int wrappedX = ((x % this->width) + this->width) % this->width;

                return wrappedX * this->height + y;
            };

            auto setVertex = [this](int pos, glm::vec3 vertex, bool left) -> void {
                if (left)
                {
                    // set left
                    this->verticesLeft[pos + 0] = vertex.x;
                    this->verticesLeft[pos + 1] = vertex.y;
                    this->verticesLeft[pos + 2] = vertex.z;
                }
                else
                {
                    // set right
                    this->verticesRight[pos + 0] = vertex.x;
                    this->verticesRight[pos + 1] = vertex.y;
                    this->verticesRight[pos + 2] = vertex.z;
                }
            };

            for( int x = 0; x < width; x++)
            {
                float phi = static_cast<float>(x) / static_cast<float>(width) * 2 * M_PI;

                glm::vec3 camerapos = getViewcirclePosition(phi);
                for( int y = 0; y < (height+1); y++)
                {
                    float theta = static_cast<float>(y) / static_cast<float>(height) * M_PI;
                    glm::vec3 cameradir = getViewcircleDirection(phi, theta);

                    glm::vec3 vertexLeft = camerapos + radius * cameradir;
                    glm::vec3 vertexRight = -camerapos + radius * cameradir;
                     // top-left
                    int vertexpos = getVertexPos(x, y) * 4;
                    if(vertexpos >= 0)
                    {
                        setVertex(vertexpos * 3 + 0 * 3, vertexLeft, true);
                        setVertex(vertexpos * 3 + 0 * 3, vertexRight, false);
                        verticesSpherical[vertexpos * 2 + 0 * 2 + 0] = phi;
                        verticesSpherical[vertexpos * 2 + 0 * 2 + 1] = theta;
                        verticesIndices[vertexpos * 2 + 0 * 2 + 0] = x;
                        verticesIndices[vertexpos * 2 + 0 * 2 + 1] = y;
                    }

                    // top-right
                    vertexpos = getVertexPos(x-1, y) * 4;
                    if(vertexpos >= 0)
                    {
                        setVertex( vertexpos * 3 + 1 * 3, vertexLeft, true );
                        setVertex( vertexpos * 3 + 1 * 3, vertexRight, false );
                        verticesSpherical[vertexpos * 2 + 1 * 2 + 0] = phi + (x == 0 ? 2*M_PI : 0.0f);
                        verticesSpherical[vertexpos * 2 + 1 * 2 + 1] = theta;
                        verticesIndices[vertexpos * 2 + 1 * 2 + 0] = (((x-1) % width) + width) % width;
                        verticesIndices[vertexpos * 2 + 1 * 2 + 1] = y;
                    }

                    // bottom-left
                    vertexpos = getVertexPos(x, y-1) * 4;
                    if(vertexpos >= 0)
                    {
                        setVertex( vertexpos * 3 + 2 * 3, vertexLeft, true );
                        setVertex( vertexpos * 3 + 2 * 3, vertexRight, false );
                        verticesSpherical[vertexpos * 2 + 2 * 2 + 0] = phi;
                        verticesSpherical[vertexpos * 2 + 2 * 2 + 1] = theta;
                        verticesIndices[vertexpos * 2 + 2 * 2 + 0] = x;
                        verticesIndices[vertexpos * 2 + 2 * 2 + 1] = y-1;
                    }

                    // bottom-right
                    vertexpos = getVertexPos(x-1, y-1) * 4;
                    if(vertexpos >= 0)
                    {
                        setVertex( vertexpos * 3 + 3 * 3, vertexLeft, true );
                        setVertex( vertexpos * 3 + 3 * 3, vertexRight, false );
                        verticesSpherical[vertexpos * 2 + 3 * 2 + 0] = phi + (x == 0 ? 2*M_PI : 0.0f);
                        verticesSpherical[vertexpos * 2 + 3 * 2 + 1] = theta;
                        verticesIndices[vertexpos * 2 + 3 * 2 + 0] = (((x-1) % width) + width) % width;
                        verticesIndices[vertexpos * 2 + 3 * 2 + 1] = y-1;
                    }

                    if (y == height)
                        continue;

                    vertexpos = getVertexPos(x,y) * 4;
                    indices[x * height * 6 + y * 6 + 0] = vertexpos + 0;
                    indices[x * height * 6 + y * 6 + 1] = vertexpos + 2;
                    indices[x * height * 6 + y * 6 + 2] = vertexpos + 1;
                    indices[x * height * 6 + y * 6 + 3] = vertexpos + 1;
                    indices[x * height * 6 + y * 6 + 4] = vertexpos + 2;
                    indices[x * height * 6 + y * 6 + 5] = vertexpos + 3;
                }
            }
            break;
        }

        dimensions = {width, height};

        GLint currentVao;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);

        leftODS.addVbo(verticesLeft, 3, GL_FLOAT, GL_FALSE, false);
        leftODS.addVbo(verticesIndices, 2, GL_FLOAT);
        leftODS.addVbo(verticesSpherical, 2, GL_FLOAT);
        leftODS.setEbo(indices);

        rightODS.addVbo(verticesRight, 3, GL_FLOAT, GL_FALSE, false);
        rightODS.addVbo(verticesIndices, 2, GL_FLOAT);
        rightODS.addVbo(verticesSpherical, 2, GL_FLOAT);
        rightODS.setEbo(indices);

        initialized = true;
    }

    void ODSSphereObject::setVerticesPositions(float radius)
    {
        if (this->mode != Mode::POINTCLOUD)
            return;

        auto getVertexPos = [this](int x, int y) -> int {
            if (y < 0 || y >= this->height)
                return -1;
            int wrappedX = ((x % this->width) + this->width) % this->width;

            return wrappedX * this->height + y;
        };

        auto setVertex = [this](int pos, glm::vec3 vertex, bool left) -> void {
            if (left)
            {
                // set left
                this->verticesLeft[pos + 0] = vertex.x;
                this->verticesLeft[pos + 1] = vertex.y;
                this->verticesLeft[pos + 2] = vertex.z;
            }
            else
            {
                // set right
                this->verticesRight[pos + 0] = vertex.x;
                this->verticesRight[pos + 1] = vertex.y;
                this->verticesRight[pos + 2] = vertex.z;
            }
        };

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < (height + 1); y++)
            {
                glm::vec3 cameradir, vertexLeft, vertexRight;

                float phi, theta;

                int vertexpos = getVertexPos(x, y) * 4;
                if (vertexpos >= 0)
                {
                    // top-left
                    phi = static_cast<float>(x - overlap) / static_cast<float>(width) * 2 * M_PI;
                    theta = static_cast<float>(y - overlap) / static_cast<float>(height) * M_PI;
                    vertexLeft = getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    vertexRight = -getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    setVertex(vertexpos * 3 + 0 * 3, vertexLeft, true);
                    setVertex(vertexpos * 3 + 0 * 3, vertexRight, false);
                    verticesSpherical[vertexpos * 2 + 0 * 2 + 0] = phi;
                    verticesSpherical[vertexpos * 2 + 0 * 2 + 1] = theta;

                    // top-right
                    phi = static_cast<float>(((x+1)%width) + overlap) / static_cast<float>(width) * 2 * M_PI;
                    theta = static_cast<float>(y - overlap) / static_cast<float>(height) * M_PI;
                    vertexLeft = getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    vertexRight = -getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    setVertex(vertexpos * 3 + 1 * 3, vertexLeft, true);
                    setVertex(vertexpos * 3 + 1 * 3, vertexRight, false);
                    verticesSpherical[vertexpos * 2 + 1 * 2 + 0] = phi;
                    verticesSpherical[vertexpos * 2 + 1 * 2 + 1] = theta;

                    // bottom-left
                    phi = static_cast<float>(x - overlap) / static_cast<float>(width) * 2 * M_PI;
                    theta = static_cast<float>(y + 1.0f + overlap) / static_cast<float>(height) * M_PI;
                    vertexLeft = getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    vertexRight = -getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    setVertex(vertexpos * 3 + 2 * 3, vertexLeft, true);
                    setVertex(vertexpos * 3 + 2 * 3, vertexRight, false);
                    verticesSpherical[vertexpos * 2 + 2 * 2 + 0] = phi;
                    verticesSpherical[vertexpos * 2 + 2 * 2 + 1] = theta;

                    // bottom-right
                    phi = static_cast<float>(((x+1)%width) + overlap) / static_cast<float>(width) * 2 * M_PI;
                    theta = static_cast<float>(y + 1.0f + overlap) / static_cast<float>(height) * M_PI;
                    vertexLeft = getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    vertexRight = -getViewcirclePosition(phi) + radius * getViewcircleDirection(phi, theta);
                    setVertex(vertexpos * 3 + 3 * 3, vertexLeft, true);
                    setVertex(vertexpos * 3 + 3 * 3, vertexRight, false);
                    verticesSpherical[vertexpos * 2 + 3 * 2 + 0] = phi;
                    verticesSpherical[vertexpos * 2 + 3 * 2 + 1] = theta;
                }
            }
        }

        leftODS.updateVBO(0, verticesLeft, verticesLeft.size());
        leftODS.updateVBO(2, verticesSpherical, verticesSpherical.size());
        rightODS.updateVBO(0, verticesRight, verticesRight.size());
        rightODS.updateVBO(2, verticesSpherical, verticesSpherical.size());
    }


    glm::vec3 ODSSphereObject::getViewcirclePosition(float phi) const
    {
        return ipd * glm::vec3{cos(phi), 0, sin(phi)};
    }

    glm::vec3 ODSSphereObject::getViewcircleDirection(float phi, float theta) const
    {
        return {-sin(phi) * sin(theta), cos(theta), cos(phi) * sin(theta)};
    }

    void ODSSphereObject::changeOverlap(double change)
    {
        overlap = std::max(-0.5, overlap + 0.01f * change);
        std::cout << "overlap is now: " << overlap << std::endl;
    }

} // namespace msi_vr
