#version 330 core

#define MEGATEXTURE true

out vec4 color;

in vec2 spherical;
in vec2 texCoord;
in float vertexDepth;

uniform int leftEye;
uniform vec2 minMaxDiscardDepth;

uniform sampler2D tex;
uniform sampler2D depth;

void main()
{
    color = vec4(texture(tex, texCoord).rgb, 1.0);
    // color = vec4(1.0,1.0,0.0,1.0);
}