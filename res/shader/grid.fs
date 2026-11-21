#version 460 core

in vec2 frag_coord;
in vec2 texture_coord;

out vec4 FragColor;

uniform int uGridWidth;
uniform int uGridHeight;

void main()
{
        vec3 color1 = vec3(40 / 255.0f);
        vec3 color2 = vec3(60 / 255.0f);

        vec2 size = vec2(uGridWidth, uGridHeight);
        vec2 pos = floor(texture_coord * size);
        float pattern_mask = mod(pos.x + pos.y, 2.0);
        vec3  color = mix(color1, color2, pattern_mask);
        FragColor = vec4(color, 1.0f);
}
