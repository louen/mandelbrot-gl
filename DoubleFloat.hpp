/// Double precision emulation : represent a `double` with two `float`s. Aka FloatFloat
struct DoubleFloat
{
    union
    {
        struct
        {
            float high;
            float low;
        };
        float values[2];
    };

    DoubleFloat()  {} /// Create an uninitialized FloatFloat
    DoubleFloat( float a, float b ) : high(a), low(b) {} /// Create a FloatFloat from two values
    DoubleFloat ( double x ) { fromDouble(x);} /// Create a FloatFloat from a double

    /// Initialize the float values from a double precision value
    inline void fromDouble( double x )
    {
        high = static_cast<float>(x); // Cast to float, losing precision
        const double highd = static_cast<double>(high); // cast back to double
        low = static_cast<float>(x - highd); // store the remainder
    }

    /// convert the value back to a double
    double toDouble() const
    {
        return static_cast<double>(high) + static_cast<double>(low);
    }

    inline void fromFloat( float x )
    {
        high = x;
        low = 0.f;
    }

    inline float toFloat() const
    {
        return high; // rounding
    }
};

DoubleFloat ff_neg( DoubleFloat x )
{
    return DoubleFloat( -x.high, -x.low );
}

std::string float2hex( float x );
#define CHECK(x) std::cout<< STRINGIFY(x)<<": "<< x << " " << float2hex( x )<<std::endl
DoubleFloat ff_add( DoubleFloat a, DoubleFloat b )
{
    CHECK( a.high );
    CHECK( a.low );
    CHECK( b.high );
    CHECK( b.low );


    const float r = a.high + b.high; CHECK( r );
    const float e = r - a.high; CHECK( e );
    const float s = ((b.high - e)
        + (a.high - (r - e)))
        + a.low + b.low; CHECK( s );

    const float h = r + s; CHECK( h );
    const float l = s - (h - r); CHECK( l );
    return DoubleFloat( h, l );
}

DoubleFloat ff_mul( DoubleFloat a, DoubleFloat b )
{
    
    const float split = 8193.0; // = 2^13 + 1

    const float ca = split * a.high;
    const float cb = split * b.high;

    const float v1a = ca - (ca - a.high);
    const float v1b = cb - (cb - b.high);

    const float v2a = a.high - v1a;
    const float v2b = b.high - v1b;

    const float c11 = a.high * b.high ; // products of the high parts
    const float c21 = v2a * v2b + (v2a * v1b + (v1a * v2b + ( v1a * v1b - c11)));

    const float c2 = a.high * b.low + a.low * b.high; // cross-products

    const float r = c11 + c2;
    const float e = r - c11;
    const float s = a.low  * b.low + ( (c2 -e ) + (c11 - (r - e)) + c21);

    const float h = r + s;
    const float l = s - (h - r);
    return DoubleFloat( h, l );
}

int ff_cmp( DoubleFloat a, DoubleFloat b )
{
    if ( a.high < b.high ) { return -1; }
    else if ( a.high == b.high )
    {
        if ( a.low < b.low ) { return -1; }
        else if ( a.low == b.low ) { return 0; }
        else { return 1; }
    }
    else { return 1; }
}

struct ComplexDoubleFloat
{
    DoubleFloat real;
    DoubleFloat im;
    ComplexDoubleFloat( DoubleFloat r, DoubleFloat i ) : real(r), im(i) {}
};

ComplexDoubleFloat cff_add( ComplexDoubleFloat a, ComplexDoubleFloat b )
{
    return ComplexDoubleFloat(
        ff_add( a.real, b.real ),
        ff_add( a.im, b.im )
    );
}

ComplexDoubleFloat cff_mul( ComplexDoubleFloat a, ComplexDoubleFloat b )
{
    return ComplexDoubleFloat(
        ff_add( ff_mul( a.real, b.real ), ff_neg( ff_mul( a.im, b.im ) ) ),
        ff_add( ff_mul( a.real, b.im ), ff_mul( a.im, b.real )));

}

ComplexDoubleFloat cff_scale( ComplexDoubleFloat a, ComplexDoubleFloat scale )
{
    return ComplexDoubleFloat(
        ff_mul( a.real, scale.real ),
        ff_mul( a.im, scale.im )
    );
}

DoubleFloat cff_norm( ComplexDoubleFloat a )
{
    ComplexDoubleFloat conj( a.real, ff_neg(a.im) );
    return cff_mul( a, conj ).real;
}
