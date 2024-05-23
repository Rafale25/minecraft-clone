#version 460 core
#extension GL_ARB_bindless_texture : require

in vec3 a_position;
in vec2 a_uv;
in int a_orientation;

out vec2 f_uv;
out int f_orientation;
flat out int f_faceId;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform vec3 u_chunkPos;

void main()
{
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(u_chunkPos + a_position, 1.0);

    f_uv = a_uv;
    f_orientation = a_orientation;
    f_faceId = gl_VertexID / 6;
    gl_Position = position;
}
