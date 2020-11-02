#include "mouse.hpp"

namespace msi_vr
{
    double Mouse::oldPosition[2] = {0.0, 0.0};
    double Mouse::position[2] = {0.0, 0.0};

    std::map<int, bool> Mouse::pressed = {};

    std::vector<std::function<void(double, double, int)>> Mouse::clickCallbacks = {};
    std::vector<std::function<void(double, double, int)>> Mouse::releasedCallbacks = {};
    std::vector<std::function<void(double, double, int)>> Mouse::dragCallbacks = {};
    std::vector<std::function<void(double, double)>> Mouse::scrollCallbacks = {};

    void Mouse::mouse_callback(GLFWwindow *window, double xpos, double ypos)
    {
        oldPosition[0] = position[0];
        oldPosition[1] = position[1];

        position[0] = xpos;
        position[1] = ypos;

        double direction[2] = {position[0] - oldPosition[0], position[1] - oldPosition[1]};
        for (auto &button : pressed)
        {
            if (button.second)
            {
                for (auto &fnc : dragCallbacks)
                {
                    fnc(direction[0], direction[1], button.first);
                }
            }
        }
    }

    void Mouse::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        switch (action)
        {
        case GLFW_PRESS:
            pressed[button] = true;
            for (auto &fnc : clickCallbacks)
                fnc(position[0], position[1], button);
            break;
        case GLFW_RELEASE:
            pressed[button] = false;
            for (auto &fnc : releasedCallbacks)
                fnc(position[0], position[1], button);
            break;
        }
    }

    void Mouse::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        for(auto &fnc : scrollCallbacks)
        {
            fnc(xoffset, yoffset);
        }
    }

    void Mouse::addClickCallback(std::function<void(double, double, int)> const &fnc)
    {
        clickCallbacks.push_back(fnc);
    }

    void Mouse::addReleasedCallback(std::function<void(double, double, int)> const &fnc)
    {
        releasedCallbacks.push_back(fnc);
    }

    void Mouse::addDragCallback(std::function<void(double, double, int)> const &fnc)
    {
        dragCallbacks.push_back(fnc);
    }

    void Mouse::addScrollCallback(std::function<void(double, double)> const &fnc)
    {
        scrollCallbacks.push_back(fnc);
    }

} // namespace msi_vr