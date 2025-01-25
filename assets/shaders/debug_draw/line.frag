#version 460 core

in vec3 v_color;

out vec4 fragColor;

void main()
{
    gl_FragDepth = gl_FragCoord.z - 0.0001; // Make sure debug geometry gets renderer on top of other geometry if drawn at the same place
    fragColor = vec4(v_color, 1.0);
}
