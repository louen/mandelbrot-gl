#version 330 core
layout (location = 0) in vec3 aPos;
out vec2 pos;
// No frills vertex shader. Just pass along the vertex position.
void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
    pos = aPos.xy;
}