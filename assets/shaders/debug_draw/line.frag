#version 460 core

in vec3 v_color;
in vec3 v_worldPosition;

// out vec4 fragColor;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

void main()
{
    gl_FragDepth = gl_FragCoord.z - 0.0001; // Make sure debug geometry gets renderer on top of other geometry if drawn at the same place
    FragColor = vec4(v_color, 1.0);
    gPosition = v_worldPosition;
}
