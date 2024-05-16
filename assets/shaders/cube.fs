#version 460 core

in vec2 f_uv;

out vec4 FragColor;

void main()
{
    FragColor = vec4(f_uv, 0.0, 1.0);
}
