#version 330 core

#define MEGATEXTURE true

out vec4 color;

in vec2 spherical;
in vec2 texCoord;
in float vertexDepth;
in float depthDiscard;

uniform int leftEye;
uniform vec3 minMaxDiscardDepth;

uniform sampler2D tex;
uniform sampler2D depth;

void main()
{
    //float mult = (sin(phi*4) + 1.0) / 2.0;
    //vec3 posabs = vec3(abs(pos.x), abs(pos.y), abs(pos.z));
    //color = max(0.0, 1.0 - distance / 20.0f) * posabs;
    //color = vec3(0.0, 1.0, 0.0) * mult;

    vec2 depthCoord = spherical;
    if(MEGATEXTURE)
    {
        depthCoord.x = 4.0 / 9.0 + 1.0 / 9.0 * depthCoord.x;
        depthCoord.y = 1.0 / 4.0 * depthCoord.y;

        if(leftEye == 0)
        {
            depthCoord.y = 2.0 / 4.0 + depthCoord.y;
        }
    }
    
    // if(depthDiscard > 0.0) discard;

    float texDepth = minMaxDiscardDepth.x + (minMaxDiscardDepth.y - minMaxDiscardDepth.x) * texture(depth, depthCoord).x;
    // if(abs(texDepth - vertexDepth) > 0.16) discard;
    

    color = texture(tex, texCoord);
}