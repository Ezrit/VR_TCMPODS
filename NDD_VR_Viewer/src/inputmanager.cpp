#include "inputmanager.hpp"
#include <iostream>

namespace msi_vr
{
    void InputManager::update(float deltaTime)
    {
        if(window == NULL) return;
        for(auto &it : updateKeyBindings)
        {
            if(glfwGetKey(window, it.first))
            {
                it.second();
            }
        }
    }

    void InputManager::callback(int key)
    {
        if(window == NULL) return;

        auto keyCallback = callbackKeyBindings.find(key);

        if(keyCallback != callbackKeyBindings.end())
        {
            keyCallback->second();
        }
    }

    bool InputManager::setWindow(GLFWwindow * newWindow)
    {
        window = newWindow;
        this->callbackKeyBindings[Key::ESCAPE] = [this]() {glfwSetWindowShouldClose(this->window, true); std::cout << "should close " << std::endl;};
        glfwSetWindowUserPointer(this->window, this);
        glfwSetKeyCallback(this->window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                InputManager* im = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
                if(action == GLFW_PRESS)
                {
                    im->callback(key);
                }
            });
        return true;
    }

    InputManager::InputManager()
        : window(NULL)
    {
    }
} // namespace msi_vr