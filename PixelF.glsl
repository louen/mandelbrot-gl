#version 420 core

#include <ColorSchemes.glsl>

in vec2 pos;
out vec4 FragColor;

uniform vec2 center = vec2(-0.5,0);
uniform float scale = 2;
uniform float ratio = 1;
uniform uint max = 1000u;

void main()
{
    vec2 p =  pos * vec2(scale*ratio, scale) + center;
    vec2 c = p;
    //Max number of iterations will arbitrarily be defined as 100. Finer detail with more computation will be found for larger values.
    vec4 color = vec4(0,0,0,0);
	for(uint i = 0u; i < max; i++)
    {
		//Perform complex number arithmetic
		p= vec2(p.x * p.x - p.y * p.y, 2.0 * p.x * p.y) + c;

		if (dot(p,p)>4.0){
			//The point, c, is not part of the set, so smoothly color it.
			color = colorScheme(i, max, dot(p,p), 2.0);
			break;
		}
	}

	FragColor = color;
  }
