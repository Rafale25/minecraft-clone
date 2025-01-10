#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec2 u_resolution;
uniform float u_FOV;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*u_resolution.xy) / u_resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;

    FragColor = vec4(color, 1.0);
}
