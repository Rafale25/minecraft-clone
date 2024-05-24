#version 460 core
#extension GL_ARB_bindless_texture : require

in vec3 a_position;
in vec2 a_uv;
in float a_orientation;

out vec3 frag_pos;
out vec2 f_uv;
flat out int f_orientation;
flat out int f_faceId;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform vec3 u_chunkPos;

void main()
{
    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(world_pos, 1.0);

    frag_pos = world_pos;
    f_uv = a_uv;
    f_orientation = int(a_orientation);
    f_faceId = gl_VertexID / 6;
    gl_Position = position;
}
