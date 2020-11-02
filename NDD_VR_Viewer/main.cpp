#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "stereocamera.hpp"
#include "inputmanager.hpp"
#include "shader.hpp"
#include "odssphereobject.hpp"
#include "mouse.hpp"
#include "texture.hpp"
#include "videosynchronizer.hpp"
#include "vrcontext.hpp"
#include "renderpass.hpp"

#include "nddapplication.hpp"

#include "quad.hpp"

using namespace msi_vr;

int main()
{
    NDDApplication appl;
    appl.loadScene("assets/scenes/cave");

    appl.run();
    
}