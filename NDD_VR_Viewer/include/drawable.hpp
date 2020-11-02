#ifndef __MSI_VR__DRAWABLE_HPP__
#define __MSI_VR__DRAWABLE_HPP__

#include <vector>

namespace msi_vr
{
    class Drawable
    {
    public:
        Drawable() {}
        virtual ~Drawable() {}

        virtual void draw() const = 0;

    protected:
    private:
    };
} // namespace msi_vr

#endif