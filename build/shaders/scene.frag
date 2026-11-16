#version 330 core

// Passo 1 do visual Obra Dinn: iluminação simples em TONS DE CINZA.
// A conversão para 2 cores acontece depois, em post.frag.

in vec3 vNormal;

uniform vec3  uLightDir;   // direção PARA a luz (normalizada)
uniform float uAlbedo;     // brilho base do objeto (0..1)
uniform float uHighlight;  // 1.0 = objeto sob o mouse

out vec4 FragColor;

void main() {
    float diffuse = max(dot(normalize(vNormal), normalize(uLightDir)), 0.0);
    float light   = 0.25 + 0.75 * diffuse;   // ambiente + difusa

    float gray = uAlbedo * light;
    gray = mix(gray, 1.0, 0.45 * uHighlight);  // realce ao passar o mouse

    FragColor = vec4(vec3(clamp(gray, 0.0, 1.0)), 1.0);
}
