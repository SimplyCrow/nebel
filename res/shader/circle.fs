#version 460 core

in vec2 frag_coord;
in vec2 texture_coord;

out vec4 FragColor;

#define RADIUS float(0.5f)
#define FADE   float(0.025f)

void main()
{
        vec2 uv = texture_coord - vec2(0.5f);
        float uvLength = length(uv);
        if(uvLength > RADIUS) {
                discard;
        }

        float alpha = 1.0f - smoothstep(RADIUS - FADE, RADIUS, uvLength);
        FragColor = vec4(1.0f, 0.0f, 0.0f, alpha);
}
