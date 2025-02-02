#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

in vec3 v_worldPosition;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    gPosition = v_worldPosition;
};
