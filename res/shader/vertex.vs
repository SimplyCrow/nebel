#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTextureCoord;

out vec2 frag_coord;
out vec2 texture_coord;

uniform vec2 uAnchor;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
        vec4 world_position = uModel * vec4(aPosition - uAnchor, 0.0f, 1.0f);
        frag_coord    = vec2(world_position);
        texture_coord = aTextureCoord;
        gl_Position = uProjection * uView * world_position;
}
