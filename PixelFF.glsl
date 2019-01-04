#version 400 core
#include <FloatFloat.glsl>
in vec2 pos;
layout( location = 0 ) out vec4 FragColor;
uniform vec4 center = vec4(-0.5,0,0,0);
uniform vec2 scale = vec2(2,0);
uniform float ratio = 1.0;
uniform uint max = 1000u;

void main()
{
    vec4 scale2D = vec4( ratio * scale, scale );
    vec4 pos_cff = cff_from_cf( pos );
    vec4 scaled_pos = cff_scale( pos_cff, scale2D );

	vec4 p = cff_add( scaled_pos , center);
    vec4 c = p;

    //Max number of iterations will arbitrarily be defined as 100. Finer detail with more computation will be found for larger values.
    vec3 color = vec3(0,0,0);
	for(uint i = 0u; i < max; i++)
    {
		//Perform complex number arithmetic
		p = cff_add(cff_mul(p,p),c);
		vec2 sqMax = ff_from_float(4.0);
		if ( ff_cmp(cff_norm(p) , sqMax ) > 0 )
        {
			//The point, c, is not part of the set, so smoothly color it. colorRegulator increases linearly by 1 for every extra step it takes to break free.
			precise float colorRegulator = float(i-1u)-log(((log(dot(p,p)))/log(2.0)))/log(2.0);
			//This is a coloring algorithm I found to be appealing. Written in HSV, many functions will work.
			color = vec3(0.95 + .012*colorRegulator , 1.0, .2+.4*(1.0+sin(.3*colorRegulator)));
			break;
		}
	}

	//Change color from HSV to RGB.
	//Algorithm from https://gist.github.com/patriciogonzalezvivo/114c1653de9e3da6e1e3
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 m = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
	FragColor = vec4(color.z * mix(K.xxx, clamp(m - K.xxx, 0.0, 1.0), color.y),1.0);
}
