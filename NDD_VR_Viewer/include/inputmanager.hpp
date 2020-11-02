#ifndef __MSI_VR__INPUTMANAGER_HPP__
#define __MSI_VR__INPUTMANAGER_HPP__

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include <GLFW/glfw3.h>

namespace msi_vr
{
    class InputManager
    {
        public:
        InputManager();
        virtual void update(float deltaTime);
        virtual void callback(int key);
        bool setWindow(GLFWwindow * window);

        enum Key
        {
            ESCAPE = GLFW_KEY_ESCAPE,
            SPACE = GLFW_KEY_SPACE,

            Q = GLFW_KEY_Q,
            W = GLFW_KEY_W,
            E = GLFW_KEY_E,
            R = GLFW_KEY_R,
            T = GLFW_KEY_T,
            Z = GLFW_KEY_Z,
            U = GLFW_KEY_U,
            I = GLFW_KEY_I,
            O = GLFW_KEY_O,
            P = GLFW_KEY_P,

            A = GLFW_KEY_A,
            S = GLFW_KEY_S,
            D = GLFW_KEY_D,
            F = GLFW_KEY_F,
            G = GLFW_KEY_G,
            H = GLFW_KEY_H,
            J = GLFW_KEY_J,
            K = GLFW_KEY_K,
            L = GLFW_KEY_L,

            Y = GLFW_KEY_Y,
            X = GLFW_KEY_X,
            C = GLFW_KEY_C,
            V = GLFW_KEY_V,
            B = GLFW_KEY_B,
            N = GLFW_KEY_N,
            M = GLFW_KEY_M
        };

        std::unordered_map<int, std::function<void()>> updateKeyBindings;
        std::unordered_map<int, std::function<void()>> callbackKeyBindings;

        private:

        GLFWwindow * window = NULL;
    };
}

#endif