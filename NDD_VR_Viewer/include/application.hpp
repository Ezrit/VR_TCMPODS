#ifndef __MSI_VR__APPLICATION_HPP__
#define __MSI_VR__APPLICATION_HPP__

#include <string>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mouse.hpp"
#include "inputmanager.hpp"

namespace msi_vr
{
    class Application
    {
    public:
        std::chrono::duration<float> fixedUpdateTime = std::chrono::milliseconds(10);

        Application(GLFWwindow* window = initWindow(2058, 1024, "Application"));
        virtual ~Application();

        void run();
        static GLFWwindow* initWindow(int windowWidth, int windowHeight, std::string const &name="Application");
        bool initInputManager(GLFWwindow *w);
        virtual void handleInput(std::chrono::duration<float> const &time);

        virtual void update(std::chrono::duration<float> const &time) = 0;
        virtual void fixedUpdate(std::chrono::duration<float> const &time) = 0;
        virtual void render() const = 0;

    protected:

        InputManager inputManager;
        GLFWwindow *window=NULL;


    private:
    };
} // namespace msi_vr

#endif