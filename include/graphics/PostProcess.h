#pragma once

#include "../glad/glad.h"
#include "../graphics/Shader.h"

#include <glm/glm.hpp>

// Renderiza a cena num framebuffer de BAIXA resolução e depois desenha esse
// resultado na janela com dithering 1-bit (estilo Obra Dinn) e escala inteira.
class PostProcess {
public:
    PostProcess(int width, int height);
    ~PostProcess();

    PostProcess(const PostProcess&) = delete;
    PostProcess& operator=(const PostProcess&) = delete;

    bool isValid() const { return m_valid; }
    int  width() const { return m_width; }
    int  height() const { return m_height; }

    // Passo 1: chame antes de desenhar a cena (liga o FBO e limpa)
    void beginScene() const;

    // Passo 2: desenha o FBO na janela aplicando o shader de dithering
    void present(const Shader& shader, int windowW, int windowH,
                 const glm::vec3& lightColor, const glm::vec3& darkColor) const;

    // Converte posição do mouse (pixels da janela, origem no topo esquerdo)
    // para pixels da imagem de baixa resolução. Retorna false se estiver nas barras pretas.
    bool windowToScene(double x, double y, int windowW, int windowH, glm::vec2& out) const;

private:
    struct Viewport {
        int x, y, w, h, scale;
    };
    Viewport computeViewport(int windowW, int windowH) const;

    int m_width;
    int m_height;
    unsigned int m_fbo = 0;
    unsigned int m_colorTex = 0;
    unsigned int m_depthRbo = 0;
    unsigned int m_vao = 0;  // VAO vazio: o triângulo de tela cheia vem do gl_VertexID
    bool m_valid = false;
};
