#version 400 core
#include <FloatFloat.glsl>
layout (location = 0) in vec3 aPos;
out vec2 pos;
uniform vec4 center = vec4(-0.5,0,0,0);
uniform vec2 scale = vec2(2,0);
uniform float ratio = 1.0;
void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
    pos = aPos.xy;
}