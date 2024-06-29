#version 460 core
#extension GL_ARB_bindless_texture : require

in vec3 a_position;
in vec2 a_uv;
in float a_orientation;

out VS_OUT {
    out vec3 frag_pos;
    out vec2 uv;
    flat out int orientation;
    flat out int faceId;
    out vec4 FragPosLightSpace;
} vs_out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform vec3 u_chunkPos;
uniform mat4 u_lightSpaceMatrix;

void main()
{
    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(world_pos, 1.0);

    vs_out.FragPosLightSpace = u_lightSpaceMatrix * vec4(world_pos, 1.0);
    vs_out.frag_pos = world_pos;
    vs_out.uv = a_uv;
    vs_out.orientation = int(a_orientation);
    vs_out.faceId = gl_VertexID / 4; // TODO: don't forget to change
    gl_Position = position;
}
