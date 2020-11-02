#version 330 core

#define PI 3.1415926538
#define MEGATEXTURE true
#define IPD 0.065

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexIndex;
layout(location = 2) in vec2 vertexSpherical;

out vec2 spherical;
out vec2 texCoord;
out float vertexDepth;

uniform sampler2D tex;
uniform sampler2D depth;

uniform int leftEye;
uniform vec3 minMaxDiscardDepth;

uniform ivec2 dimension;
uniform mat4 modelmatrix;
uniform mat4 viewmatrix;
uniform mat4 projectionmatrix;


vec3 getViewcirclePosition(float phi)
{
    return IPD * vec3(cos(phi), 0, sin(phi));
}

vec3 getViewcircleDirection(float phi, float theta)
{
    return vec3(-sin(phi) * sin(theta), cos(theta), cos(phi) * sin(theta));
}

void main()
{
    texCoord = vec2(vertexSpherical.x / (2*PI), vertexSpherical.y / PI);
    vec2 depthCoord = vec2(float(vertexIndex.x+0.5f) / float(dimension.x), float(vertexIndex.y+0.5f) / float(dimension.y));

    if(MEGATEXTURE)
    {
        texCoord.x = 4.0 / 9.0 * texCoord.x;
        depthCoord.x = 4.0 / 9.0 + 1.0 / 9.0 * depthCoord.x;
        depthCoord.y = 1.0 / 4.0 * depthCoord.y;

        if(leftEye == 0)
        {
            texCoord.x = texCoord.x + 5.0 / 9.0;
            depthCoord.y = depthCoord.y + 2.0 / 4.0;
        }
    }

    vertexDepth = minMaxDiscardDepth.x + (minMaxDiscardDepth.y - minMaxDiscardDepth.x) * texture(depth, depthCoord).x;
    vec4 depthPos = vec4(getViewcirclePosition(vertexSpherical.x), 1.0) + vec4(vertexDepth * getViewcircleDirection(vertexSpherical.x, vertexSpherical.y) , 1.0);
    if(leftEye == 0) depthPos = vec4(getViewcirclePosition(vertexSpherical.x), 1.0) + vec4(vertexDepth * getViewcircleDirection(vertexSpherical.x, vertexSpherical.y) , 1.0);

    vec4 worldvertex = modelmatrix * depthPos;
    vec4 viewVertex = viewmatrix * worldvertex;

    gl_Position = projectionmatrix * viewVertex;
}