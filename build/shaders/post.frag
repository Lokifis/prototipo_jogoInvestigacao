#version 330 core

// Passo 2 do visual Obra Dinn: dithering ordenado (Bayer 8x8) em 2 cores.

in vec2 vUV;

uniform sampler2D uScene;
uniform vec2 uResolution;  // resolução da imagem de baixa resolução
uniform vec3 uLightColor;  // cor "clara" da paleta
uniform vec3 uDarkColor;   // cor "escura" da paleta

out vec4 FragColor;

// Matriz de Bayer 8x8 calculada por bits (valores de 0 a 1)
float bayer8(ivec2 p) {
    int x = p.x & 7;
    int y = p.y & 7;
    int xy = x ^ y;
    int v = ((xy & 1) << 5) | ((y & 1) << 4)
          | ((xy & 2) << 2) | ((y & 2) << 1)
          | ((xy & 4) >> 1) | ((y & 4) >> 2);
    return (float(v) + 0.5) / 64.0;
}

void main() {
    ivec2 texel = ivec2(clamp(floor(vUV * uResolution), vec2(0.0), uResolution - 1.0));
    float gray = texelFetch(uScene, texel, 0).r;

    // TODO: testar blue noise (textura) no lugar do Bayer
    // TODO: ancorar o padrão no mundo/objeto para não "nadar" quando a câmera gira
    float threshold = bayer8(texel);
    vec3 color = (gray > threshold) ? uLightColor : uDarkColor;

    FragColor = vec4(color, 1.0);
}
