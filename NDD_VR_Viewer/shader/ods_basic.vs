#version 330 core

#define PI 3.1415926538

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in ivec2 vertexIndex;
layout(location = 2) in vec2 vertexSpherical;
layout(location = 3) in ivec2 dimension;

out vec3 pos;
out float distance;
out float phi;
out vec2 texCoord;

uniform mat4 modelmatrix;
uniform mat4 viewmatrix;
uniform mat4 projectionmatrix;

void main()
{
    vec4 worldvertex = modelmatrix * vec4(vertexPosition, 1);
    vec4 viewVertex = viewmatrix * worldvertex;

    pos = normalize(worldvertex.xyz / worldvertex.w);
    distance = length(viewVertex.xyz / viewVertex.w);
    phi = vertexSpherical.x;
    texCoord = vec2(vertexSpherical.x / (2*PI), vertexSpherical.y / PI);
    gl_Position = projectionmatrix * viewVertex;
}