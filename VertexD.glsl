#version 400 core
#include <FloatFloat.glsl>
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 cPos;
out vec4 pos;
uniform vec4 center = vec4(-0.5,0,0,0);
uniform vec2 scale = vec2(2,0);
uniform float ratio = 1.0;
void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
    /*vec4 p = cff_from_cf(aPos.xy);

	vec2 scaleX = ff_mul( ff_from_float(ratio), scale );
	vec2 scaleY = scale;
	vec4 scaleP = vec4(scaleX,scaleY);
	vec4 pp = cff_scale(p, scaleP);
	pos = cff_add(center, pp );
    */
    pos = cPos;
}