#version 460 core

in vec2 frag_coord;
in vec2 texture_coord;

out vec4 FragColor;

uniform int uGridWidth;
uniform int uGridHeight;
uniform sampler2D uHeatMap;

void main()
{
        FragColor = vec4(0.0f, 0.0f, texture(uHeatMap, texture_coord).r, 0.2f);
}
