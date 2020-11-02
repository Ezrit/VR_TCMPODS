
#version 330 core

layout(location = 0) in vec3 vertexPosition_screenspace;

out vec2 texCoord;

void main()
{
    texCoord = vec2((vertexPosition_screenspace.x + 1.0) / 2.0, (vertexPosition_screenspace.y + 1.0) / 2.0); 
    gl_Position = vec4(vertexPosition_screenspace, 1);
}