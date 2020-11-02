#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 MVP;

void main()
{
    mat4 identity = mat4(1.0);
    gl_Position = identity * MVP * vec4(vertexPosition_modelspace, 1);
}