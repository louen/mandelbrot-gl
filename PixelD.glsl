#version 400 core
#include <FloatFloat.glsl>
in vec2 pos;
out vec4 FragColor;
uniform vec4 center = vec4(-0.5,0,0,0);
uniform vec2 scale = vec2(2,0);
uniform float ratio = 1.0;
uniform uint max = 1000u;

double cf_to_double( vec2 a ) {
    return double( a.x ) + double( a.y );
}

double absd( double x )
{
    if ( x < 0 ) return -x;
    else return x;
}

void main()
{

    vec4 scaleD = vec4( scale, scale );
    vec4 posD = cff_from_cf( /*pos*/ vec2(0.1,0.1) );
    vec4 scaled = cff_scale( posD, scaleD );
	vec4 p = cff_add( scaled , center);
    vec4 c = p;


    double scaleDouble = cf_to_double( scale );
    double posX = double( /*pos.x*/0.1 );
    double posY = double( /*pos.y*/0.1 );
    double scaledX = scaleDouble * posX;
    double scaledY = scaleDouble * posY;

    double pX = scaledX + cf_to_double( center.xy );
    double pY = scaledY + cf_to_double( center.zw );

    double N = pX * pX + pY * pY;

    //Max number of iterations will arbitrarily be defined as 100. Finer detail with more computation will be found for larger values.
    vec3 color = vec3(0,0,0);
	//for(uint i = 0u; i < max; i++)
    {
		//Perform complex number arithmetic
		//p = cff_add(cff_mul(p,p),c);
		vec2 sqMax = ff_from_float(4.0);
		//if ( ff_cmp(cff_norm(p) , sqMax ) > 0 )
		//if (pX * pX + pY * pY > double(4) )

        double errX = scaledX - cf_to_double( scaled.xy );
        double errY = scaledY - cf_to_double( scaled.zw );

        double err = absd( errX );// +absd( errY );
        // err X max ~2 e-13


        vec2 ppX = ff_add( scaled.xy, center.xy );
        //vec2 ppY = ff_add( scaled.zw, center.zw );

        double errPPX = pX - cf_to_double( ppX );
        //double errPPY = pY - cf_to_double( ppY );

        double errPP = absd( errPPX );
        
        double errN = absd( N - cf_to_double( cff_norm( p ) ) );

        if ( errPP > double( 1e-8 ) ) { color = vec3( 1e7*errPP, 0, 0 ); }

        /*
        if ( N > double(4.0))
        {
			//The point, c, is not part of the set, so smoothly color it. colorRegulator increases linearly by 1 for every extra step it takes to break free.
			//float colorRegulator = float(i-1u)-log(((log(dot(p,p)))/log(2.0)))/log(2.0);
			//This is a coloring algorithm I found to be appealing. Written in HSV, many functions will work.
			//color = vec3(0.95 + .012*colorRegulator , 1.0, .2+.4*(1.0+sin(.3*colorRegulator)));
			color +=vec3(0,0,1);
			//break;
		}

        if ( cf_to_double( cff_norm( p ) ) > double( 4.0 ) )
        {
            color += vec3( 1, 0, 0 );
        }*/
	}

	//Change color from HSV to RGB.
	//Algorithm from https://gist.github.com/patriciogonzalezvivo/114c1653de9e3da6e1e3
	//vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	//vec3 m = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
	//FragColor = vec4(color.z * mix(K.xxx, clamp(m - K.xxx, 0.0, 1.0), color.y),1.0);
	FragColor = vec4(color, 1.0);
}
