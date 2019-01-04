/// Double precision emulation : represent a `double` with two `float`s. Aka FloatFloat
struct FloatFloat
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

    FloatFloat()  {} /// Create an uninitialized FloatFloat
    FloatFloat( float a, float b ) : high(a), low(b) {} /// Create a FloatFloat from two values
    FloatFloat ( double x ) { fromDouble(x);} /// Create a FloatFloat from a double

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

    /// Initialize from a single float ( low = 0 )
    inline void fromFloat( float x )
    {
        high = x;
        low = 0.f;
    }

    /// Round to nearest float (ignore low part)
    inline float toFloat() const
    {
        return high; // rounding
    }
};

FloatFloat ff_neg( FloatFloat x )
{
    return FloatFloat( -x.high, -x.low );
}

FloatFloat ff_add( FloatFloat a, FloatFloat b )
{
    const float r = a.high + b.high;
    const float e = r - a.high;
    const float s = ((b.high - e)
        + (a.high - (r - e)))
        + a.low + b.low;

    const float h = r + s;
    const float l = s - (h - r);
    return FloatFloat( h, l );
}

FloatFloat ff_mul( FloatFloat a, FloatFloat b )
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
    return FloatFloat( h, l );
}

int ff_cmp( FloatFloat a, FloatFloat b )
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