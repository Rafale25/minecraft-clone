#version 460 core

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main()
{
    vec3 vertices[3] = {
        vec3(0.0, 0.0, 0.0),
        vec3(10.0, 0.0, 0.0),
        vec3(10, 10.0, 0.0),
    };

    vec3 vert = vertices[gl_VertexID % 3];
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(vert, 1.0);

    gl_Position = position;
}
