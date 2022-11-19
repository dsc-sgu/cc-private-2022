#version 330
#define M_PI 3.1415926535897932384626433832795

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform vec2 resolution;
uniform vec4 viewport;
uniform vec2 l;
uniform vec2 r;
uniform vec2 t;
uniform vec2 b;
uniform float time;

vec2 ds_set(float a)
{
    vec2 z;
    z.x = a;
    z.y = 0.0;
    return z;
}

vec2 ds_mul (vec2 dsa, vec2 dsb)
{
    vec2 dsc;
    float c11, c21, c2, e, t1, t2;
    float a1, a2, b1, b2, cona, conb, split = 8193.;

    cona = dsa.x * split;
    conb = dsb.x * split;
    a1 = cona - (cona - dsa.x);
    b1 = conb - (conb - dsb.x);
    a2 = dsa.x - a1;
    b2 = dsb.x - b1;

    c11 = dsa.x * dsb.x;
    c21 = a2 * b2 + (a2 * b1 + (a1 * b2 + (a1 * b1 - c11)));

    c2 = dsa.x * dsb.y + dsa.y * dsb.x;

    t1 = c11 + c2;
    e = t1 - c11;
    t2 = dsa.y * dsb.y + ((c2 - e) + (c11 - (t1 - e))) + c21;

    dsc.x = t1 + t2;
    dsc.y = t2 - (dsc.x - t1);

    return dsc;
}

vec2 ds_add (vec2 dsa, vec2 dsb)
{
    vec2 dsc;
    float t1, t2, e;

    t1 = dsa.x + dsb.x;
    e = t1 - dsa.x;
    t2 = ((dsb.x - e) + (dsa.x - (t1 - e))) + dsa.y + dsb.y;

    dsc.x = t1 + t2;
    dsc.y = t2 - (dsc.x - t1);
    return dsc;
}

// Substract: res = ds_sub(a, b) => res = a - b
vec2 ds_sub (vec2 dsa, vec2 dsb)
{
    vec2 dsc;
    float e, t1, t2;

    t1 = dsa.x - dsb.x;
    e = t1 - dsa.x;
    t2 = ((-dsb.x - e) + (dsa.x - (t1 - e))) + dsa.y - dsb.y;

    dsc.x = t1 + t2;
    dsc.y = t2 - (dsc.x - t1);
    return dsc;
}

float ds_compare(vec2 dsa, vec2 dsb)
{
    if (dsa.x < dsb.x) return -1.;
    else if (dsa.x == dsb.x) 
    {
        if (dsa.y < dsb.y) return -1.;
        else if (dsa.y == dsb.y) return 0.;
        else return 1.;
    }
    else return 1.;
}

struct complex_long {
    vec2 r;
    vec2 i;
};

complex_long product(complex_long a, complex_long b)
{
    return complex_long( ds_sub(ds_mul(a.r, b.r), ds_mul(a.i, b.i))
                       , ds_add(ds_mul(a.r, b.i), ds_mul(a.i, b.r))
                       );
}

complex_long powc(complex_long c, int p)
{
    complex_long res = c;
    for (int i = 1; i < p; ++i)
        res = product(res, c);
    return res;
}

vec2 dot(complex_long a, complex_long b)
{
    return ds_add(ds_mul(a.r, b.r), ds_mul(a.i, b.i));
}

float f(float x, float q, float p)
{
    float a = cos(sqrt(x) * q + p);
    return a * a;
}

void main()
{
    // vec2 uv = fragTexCoord * (viewport.yw - viewport.xz) + viewport.xz;
    vec2 v = ds_sub(t, b);
    vec2 h = ds_sub(r, l);
    vec2 uvx = ds_add(ds_mul(ds_set(fragTexCoord.x), h), l);
    vec2 uvy = ds_add(ds_mul(ds_set(fragTexCoord.y), v), b);

    complex_long z = complex_long(ds_set(0.0), ds_set(0.0));
    complex_long c = complex_long(uvx, uvy);

    int iterations = 0;
    bool inside = true;
    for (int i = 0; i < 200; i++)
    {
        complex_long z2 = powc(z, 2);
        z = complex_long(ds_add(z2.r, c.r), ds_add(z2.i, c.i));

        iterations += 1;
        vec2 d = dot(z, z);
        if (ds_compare(d, ds_set(4.0)) > 0)
        {
            inside = false;
            break;
        }
    }

    float k = mod(time, M_PI);
    finalColor = vec4(
        f(float(iterations), 1.0, k + 0.0),
        f(float(iterations), 1.0, k + 120.0),
        f(float(iterations), 1.0, k + 240.0),
        1.0
    );
}
