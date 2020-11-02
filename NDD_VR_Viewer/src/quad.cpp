#include "quad.hpp"

#include <iostream>

namespace msi_vr
{
    Quad::Quad(std::vector<float> const &vertices)
        : GLObject()
    {
        assert(vertices.size() >= 12);
        if(vertices.size() < 12)
        {
            std::cerr << "A Quad needs 3 vertices -> 12 floats!" << std::endl;
            return;
        }

        for(int i=0; i<12; i++)
        {
            this->vertices[i] = vertices[i];
        }

        addVbo(vertices);
        setEbo({indices[0], indices[1], indices[2], indices[3], indices[4], indices[5]});
        triangleCount = 2;
    }

    Quad::~Quad()
    {
    }

} // namespace msi_vr