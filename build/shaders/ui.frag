#version 330 core
out vec4 FragColor;

uniform float uShade; // 0.0 = cor escura da paleta, 1.0 = cor clara da paleta

void main() {
    FragColor = vec4(vec3(uShade), 1.0);
}
