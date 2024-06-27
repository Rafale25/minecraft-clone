#version 330 core

in vec2 aPos;
in vec2 aUv;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
