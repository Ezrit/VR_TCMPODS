#ifndef __MSI_VR__MOUSE_HPP__
#define __MSI_VR__MOUSE_HPP__

#include <functional>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace msi_vr
{
    class Mouse
    {
        public:
        static double oldPosition[2];
        static double position[2];
        static std::map<int, bool> pressed;

        static std::vector<std::function<void(double, double, int)>> clickCallbacks;
        static std::vector<std::function<void(double, double, int)>> releasedCallbacks;
        static std::vector<std::function<void(double, double, int)>> dragCallbacks;
        static std::vector<std::function<void(double, double)>> scrollCallbacks;

        static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

        static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

        static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        
        static void addClickCallback(std::function<void(double, double, int)> const &fnc);

        static void addReleasedCallback(std::function<void(double, double, int)> const &fnc);

        static void addDragCallback(std::function<void(double, double, int)> const &fnc);

        static void addScrollCallback(std::function<void(double, double)> const &fnc);
    }; // namespace Mouse
}

#endif