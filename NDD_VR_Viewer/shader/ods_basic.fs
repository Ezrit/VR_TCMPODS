#version 330 core

out vec4 color;

in vec3 pos;
in float distance;
in float phi;
in vec2 texCoord;

uniform sampler2D tex;

void main()
{
    //float mult = (sin(phi*4) + 1.0) / 2.0;
    //vec3 posabs = vec3(abs(pos.x), abs(pos.y), abs(pos.z));
    //color = max(0.0, 1.0 - distance / 20.0f) * posabs;
    //color = vec3(0.0, 1.0, 0.0) * mult;

    color = texture(tex, texCoord);
}