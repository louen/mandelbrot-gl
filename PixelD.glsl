#version 420 core
#include <ColorSchemes.glsl>

in vec2 pos;
out vec4 FragColor;

uniform dvec2 center = dvec2(-0.5,0);
uniform double scale = 2.0;
uniform float ratio = 1.0;
uniform uint max = 1000u;

void main()
{
    dvec2 p =  dvec2(pos) * dvec2(scale*ratio, scale) + center;
    dvec2 c = p;
    vec4 color = vec4(0,0,0,0);
    for(uint i = 0u; i < max; i++)
    {
        //Perform complex number arithmetic
        p = dvec2(p.x * p.x - p.y * p.y, 2.0 * p.x * p.y) + c;

        if (dot(p,p)>4.0){
            //The point, c, is not part of the set, so smoothly color it.
            color = colorScheme(i, max, float(dot(p,p)), 2.0);
            break;
        }
    }

    FragColor = color;
}
