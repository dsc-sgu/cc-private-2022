#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform vec2 resolution;
uniform vec4 viewport;

vec2 product(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 powc(vec2 c, int p)
{
    vec2 res = c;
    for (int i = 1; i < p; ++i)
        res = product(res, c);
    return res;
}

float f(float x, float q, float p)
{
    float a = cos(sqrt(x) * q + p);
    return a * a;
}

void main()
{
    vec2 uv = fragTexCoord * (viewport.yw - viewport.xz) + viewport.xz;

    vec2 z = vec2(0.0, 0.0);
    vec2 c = uv.xy;

    int iterations = 0;
    bool inside = true;
    for (int i = 0; i < 1000; i++)
    {
        z = powc(z, 2) + c;

        iterations += 1;
        if (dot(z, z) > 4.0)
        {
            inside = false;
            break;
        }
    }

    if (inside)
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
        finalColor = vec4(
            f(float(iterations), 1.0, 0.0),
            f(float(iterations), 1.0, 120.0),
            f(float(iterations), 1.0, 240.0),
            1.0
        );
}
