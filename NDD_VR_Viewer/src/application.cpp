#include "application.hpp"

#include <iostream>

namespace msi_vr
{
    Application::Application(GLFWwindow *window)
        : window(window)
    {
        initInputManager(window);
    }

    Application::~Application()
    {
    }

    void Application::run()
    {
        std::chrono::high_resolution_clock::time_point point = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime;
        std::chrono::duration<float> accumulatedTime(0.f);

        while (glfwWindowShouldClose(window) == 0)
        {
            auto newPoint = std::chrono::high_resolution_clock::now();
            elapsedTime = newPoint - point;
            point = newPoint;
            accumulatedTime += elapsedTime;

            while (accumulatedTime > fixedUpdateTime)
            {
                inputManager.update(fixedUpdateTime.count());

                fixedUpdate(fixedUpdateTime);
                accumulatedTime -= fixedUpdateTime;
            }

            update(elapsedTime);

            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            render();
            // swap
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    GLFWwindow* Application::initWindow(int windowWidth, int windowHeight, std::string const &name)
    {
        glewExperimental = true;
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return NULL;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);                               // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                 // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // We want OpenGL 3.3
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We dont want the old OpenGL

        // Open a window and create its opengl context
        //std::unique_ptr<GLFWwindow> window = std::unique_ptr<GLFWwindow>(glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL));
        GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, name.c_str(), NULL, NULL);

        if (window == NULL)
        {
            std::cerr << "Failed to open GLFW Window!" << std::endl;
            return NULL;
        }

        glfwMakeContextCurrent(window); // Initialize GLEW
        glewExperimental = true;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            return NULL;
        }

        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

        glfwSetCursorPosCallback(window, Mouse::mouse_callback);
        glfwSetMouseButtonCallback(window, Mouse::mouse_button_callback);
        glfwSetScrollCallback(window, Mouse::mouse_scroll_callback);

        return window;
    }

    bool Application::initInputManager(GLFWwindow *w)
    {
        if(!w) return false;

        inputManager.setWindow(w);

        
        glfwSetCursorPosCallback(window, Mouse::mouse_callback);
        glfwSetMouseButtonCallback(window, Mouse::mouse_button_callback);
        return true;
    }

    void Application::handleInput(std::chrono::duration<float> const &time)
    {
        inputManager.update(time.count());
    }
} // namespace msi_vr