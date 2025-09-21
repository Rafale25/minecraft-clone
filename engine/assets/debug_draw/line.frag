#version 460 core

in vec3 v_worldPosition;
in flat int v_color;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

#include "utils/packing.glsl"

void main()
{
    gl_FragDepth = gl_FragCoord.z - 0.00001; // Make sure debug geometry gets renderer on top of other geometry if drawn at the same place
    FragColor = vec4(unpackColor(v_color), 1.0);
    gPosition = v_worldPosition;
}
