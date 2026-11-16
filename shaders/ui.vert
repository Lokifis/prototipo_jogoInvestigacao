#version 330 core
layout (location = 0) in vec2 aPos; // Quadrado unitário (0,0) até (1,1)

uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
