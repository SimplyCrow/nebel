#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aInstanceOffset;

out vec2 FragCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
        FragCoord = aPosition;
        vec4 position = vec4(aPosition + aInstanceOffset, 0.0f, 1.0f);
        gl_Position = uProjection * uView * uModel * position;
}
