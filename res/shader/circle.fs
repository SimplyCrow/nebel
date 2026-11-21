#version 460 core

in vec2 frag_coord;
in vec2 texture_coord;

out vec4 FragColor;

#define RADIUS float(0.5f)
#define FADE   float(0.025f)

vec3 rainbow(float t) {
    t = clamp(t, 0.0, 1.0);
    return 0.5 + 0.5 * cos(6.2831853 * (t + vec3(0.0, 0.3333333, 0.6666667)));
}

void main()
{
        vec2 uv = texture_coord - vec2(0.5f);
        float uvLength = length(uv);
        if(uvLength > RADIUS) {
                discard;
        }

        float alpha = 1.0f - smoothstep(RADIUS - FADE, RADIUS, uvLength);
        FragColor = vec4(1.0f, 0.0f, 0.0f, alpha); //vec4(rainbow(fract(0.5 * (uv.x + uv.y + 1.0f))), 1.0f);
}
