#ifndef __MSI_VR__QUAD_HPP__
#define __MSI_VR__QUAD_HPP__

#include "globject.hpp"

namespace msi_vr
{
    class Quad : public GLObject
    {
        public:
        Quad(std::vector<float> const &vertices);
        virtual ~Quad();

        protected:
        float vertices[12];
        unsigned int indices[6] = {0, 2, 1, 1, 2, 3};
        private:
    };
}

#endif