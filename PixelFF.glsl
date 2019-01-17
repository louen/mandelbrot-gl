#version 420 core
#pragma optionNV(fastmath off)
#pragma optionNV(fastprecision off)
#include <FloatFloat.glsl>
#include <ColorSchemes.glsl>


in vec2 pos;
out vec4 FragColor;

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

    vec4 color = vec4(0,0,0,0);
    for(uint i = 0u; i < max; i++)
    {
        //Perform complex number arithmetic
        p = cff_add(cff_mul(p,p),c);
        vec2 sqMax = ff_from_float(4.0);
        if ( ff_cmp(cff_norm(p) , sqMax ) > 0 )
        {
            color = colorScheme(i,max, dot(p,p),2.0);
            break;
        }
    }

    FragColor = color;
}
