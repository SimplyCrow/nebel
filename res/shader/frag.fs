#version 460 core

in vec2 FragCoord;

out vec4 FragColor;

#define RADIUS float(0.5f)
#define FADE   float(0.025f)

void main()
{
        vec2 uv = FragCoord;
        float uvLength = length(uv);
        if(uvLength > RADIUS) {
                discard;
        }

        float alpha = 1.0f - smoothstep(RADIUS - FADE, RADIUS, uvLength);
        FragColor = vec4(1.0f, 1.0f, 1.0f, alpha);
}
