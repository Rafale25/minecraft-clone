#version 460 core

in vec3 a_position;
in vec2 a_uv;
in int a_orientation;

out vec2 f_uv;
out int f_orientation;
out vec3 f_normal;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

const vec3 orientation_normal_table[] = { // Note: maybe should be done in fragment shader to save bandwidth
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, 1.0), // Front = 2
    vec3(0.0, 0.0, -1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

void main()
{
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(a_position, 1.0);

    f_uv = a_uv;
    f_orientation = a_orientation;
    f_normal = orientation_normal_table[a_orientation];
    gl_Position = position;
}
