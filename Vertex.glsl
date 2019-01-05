#version 330 core
layout (location = 0) in vec3 aPos;
out vec2 pos;
uniform vec2 center = vec2(-0.5,0);
uniform float scale = 2;
uniform float ratio = 1;
void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
    pos = aPos.xy;
}