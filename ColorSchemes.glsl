// Common code for coloring mandelbrot
// A collection of colorschemes I picked up from other sources

//Change color from HSV to RGB.
vec4 HSVtoRGBA(vec3 hsv)
{
    //Algorithm from https://gist.github.com/patriciogonzalezvivo/114c1653de9e3da6e1e3
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 m = abs(fract(hsv.xxx + K.xyz) * 6.0 - K.www);
	return  vec4(hsv.z * mix(K.xxx, clamp(m - K.xxx, 0.0, 1.0), hsv.y),1.0);
}


vec4 blueYellow( uint iters, uint maxiters, float radius, float maxi)
{
	float speed = float(iters)/float(maxiters);
	return vec4 ( speed, speed, 0.5, 1.0 );
}

vec4 flashyColor( uint iters, uint maxiters, float radius, float maxi )
{
	const float log2 = log(2.0);
	// colorRegulator increases linearly by 1 for every extra step it takes to break free.
	float colorRegulator = float(iters-1u)-log(((log(radius))/log2)/log2);
	//This is a coloring algorithm I found to be appealing. Written in HSV, many functions will work.
	vec3  hsv = vec3(0.95 + .012*colorRegulator , 1.0, .2+.4*(1.0+sin(.3*colorRegulator)));
	return HSVtoRGBA(hsv);
}

vec4 softColor( uint iters, uint maxiters, float radius, float maxi)
{

	const float log2 = log(2.0);
    float colorRegulator = 1. - log( 0.5*log(radius) / log2 ) / log2;
	float speed2 = log(iters+ colorRegulator);
	vec3 hsv = vec3(speed2, 0.4, 1.);
	return HSVtoRGBA(hsv);
}

vec4 colorScheme( uint iters, uint maxiters, float radius, float maxi)
{
	//return flashyColor(iters, maxiters, radius, maxi);
	//return blueYellow(iters,maxiters,radius,maxi);
	return softColor(iters,maxiters,radius,maxi);
}


