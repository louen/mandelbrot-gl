// Operations for double precision emulation with two floats (aka floatfloat)
// .x = high, and .y = low

// create a floatflat from a single float value
vec2 ff_from_float(float a)
{
    return vec2(a, 0.0);
}

// Cast a floatfloat to float
float ff_to_float( vec2 ff )
{
    return ff.x; // Leave the low part out ( rounding )
}

// Addition of 2 floatfloats (add22).
// Taking care of a potential carry
vec2 ff_add(vec2 a, vec2 b)
{
    vec2 result;
    float r = a.x + b.x; // sum the high parts
    float e = r - a.x;   // ~ b.x
    float s  = ((b.x - e)
              + (a.x - (r - e)))
              + a.y + b.y;  // sum the low parts

    // add12
    float x = r + s; // carry
    float y = s - (x - r);

    return vec2(x,y);
}

// Multiplication of 2 floatfloat
vec2 ff_mul(vec2 a, vec2 b)
{
    const float split = 8193.0; // = 2^13 + 1

    vec2 hi = vec2(a.x,b.x);
    vec2 c = split * hi;

    vec2 v1  = c - ( c  - hi);
    vec2 v2 = hi - v1;

    float c11 = a.x * b.x ; // products of the high parts
    float c21 = v2.x * v2.y + (v2.x * v1.y + (v1.x * v2.y + ( v1.x * v1.y - c11)));

    float c2 = a.x * b.y + a.y * b.x; // cross-products

    float r = c11 + c2;
    float e = r - c11;
    float s = a.y  * b.y + ( (c2 -e ) + (c11 - (r - e)) + c21);

    float x = r + s;
    float y = s - (x - r);
    return vec2(x,y);
}


// -1 if a < b
// 0 if a == b
// 1 if a > b
float ff_cmp(vec2 a, vec2 b )
{
    if ( a.x < b.x )
    {
        return -1.;
    }
    else if (a.x == b.x)
    {
        if ( a.y < b.y)
        {
            return -1.;
        }
        else if (a.y == b.y)
        {
            return 0.;
        }
        else
        {
            return 1.;
        }

    }
    else
    {
        return 1.;
    }
}
// Complex operations for vec4 floatfloat-based complex numbers
// v.xy = floatfloat real part
// v.zw = floatfloat imaginary part(

vec4 cff_from_cf(vec2 a)
{
    return vec4 (
        ff_from_float(a.x),
        ff_from_float(a.y)
    );
}

vec4 cff_add(vec4 a, vec4 b)
{
    return vec4 (
        ff_add(a.xy, b.xy),
        ff_add(a.zw, b.zw));
}

vec4 cff_mul( vec4 a, vec4 b )
{
    //   (p  + i q) *  (r + is ) = ( pr - qs + i ps + qr)
    return vec4 (
        ff_add( ff_mul(a.xy,b.xy), -ff_mul(a.zw,b.zw)),
        ff_add( ff_mul(a.xy,b.zw),  ff_mul(a.zw,b.xy))
    );
}

vec4 cff_scale(vec4 a , vec4 scale)
{
    return vec4 (
        ff_mul(a.xy, scale.xy),
        ff_mul(a.zw, scale.zw)
    );
}

vec2 cff_norm(vec4 a)
{
    vec4 conj = vec4  (a.xy, -a.zw );
    return (cff_mul(a,conj)).xy;
}

