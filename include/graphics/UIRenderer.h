#pragma once

#include "../glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <string>
#include "font8x8_basic.h"
#include "Shader.h"
using namespace std;
class UIRenderer {
public:
    UIRenderer(int virtualWidth, int virtualHeight) {
        m_projection = glm::ortho(0.0f, static_cast<float>(virtualWidth),
                                  static_cast<float>(virtualHeight), 0.0f);

        // Quadrado unitário (0,0) -> (1,1)
        const float quad[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    ~UIRenderer() {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
    }

    // Desenha um retângulo sólido (shade: 0.0 = escuro, 1.0 = claro)
    void drawRect(const Shader& shader, float x, float y, float w, float h, float shade) const {
        shader.use();
        shader.setMat4("uProjection", m_projection);
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(w, h, 1.0f));
        shader.setMat4("uModel", model);
        shader.setFloat("uShade", shade);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // Desenha uma janela/painel estilo 1-bit (fundo escuro com borda dupla clara)
    void drawPanel(const Shader& shader, float x, float y, float w, float h) const {
        drawRect(shader, x, y, w, h, 0.0f);                     // Fundo escuro
        drawRect(shader, x + 2, y + 2, w - 4, h - 4, 1.0f);     // Borda clara externa
        drawRect(shader, x + 4, y + 4, w - 8, h - 8, 0.0f);     // Miolo escuro
    }

    // Desenha texto com quebra automática de linha dentro de maxWidth
    void drawTextWrapped(const Shader& shader, const string& rawText,
                         float x, float y, float maxWidth, float scale = 1.0f, float shade = 1.0f) const {
        string text = (rawText);
        const float charAdvance = 8.0f * scale;
        const float lineHeight  = 11.0f * scale;

        istringstream words(text);
        string word;
        float cursorX = x;
        float cursorY = y;

        while (words >> word) {
            float wordWidth = word.size() * charAdvance;
            if (cursorX > x && (cursorX - x + wordWidth) > maxWidth) {
                cursorX = x;
                cursorY += lineHeight;
            }
            drawSingleLine(shader, word, cursorX, cursorY, scale, shade);
            cursorX += wordWidth + charAdvance; // + espaço
        }
    }

    // Desenha um botão interativo e retorna true se o jogador clicou nele neste frame
    bool drawButton(const Shader& shader, const string& label,
                    float x, float y, float w, float h,
                    glm::vec2 mouseScenePos, bool justClicked) const {
        bool hovered = (mouseScenePos.x >= x && mouseScenePos.x <= x + w &&
                        mouseScenePos.y >= y && mouseScenePos.y <= y + h);

        // Inverte as cores quando o mouse passa por cima (feedback visual 1-bit)
        float bgShade   = hovered ? 1.0f : 0.0f;
        float textShade = hovered ? 0.0f : 1.0f;

        drawRect(shader, x, y, w, h, 1.0f);                 // Borda
        drawRect(shader, x + 1, y + 1, w - 2, h - 2, bgShade); // Interior
        drawSingleLine(shader,(label), x + 6, y + (h - 8.0f) * 0.5f, 1.0f, textShade);

        return hovered && justClicked;
    }

private:
    glm::mat4    m_projection{1.0f};
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;

    void drawSingleLine(const Shader& shader, const std::string& text,
                        float x, float y, float scale, float shade) const {
        shader.use();
        shader.setMat4("uProjection", m_projection);
        shader.setFloat("uShade", shade);
        glBindVertexArray(m_vao);

        for (size_t c = 0; c < text.size(); ++c) {
            unsigned char ch = static_cast<unsigned char>(text[c]);
            if (ch >= 128) ch = '?';

            const char* glyph = font8x8_basic[ch];
            float baseX = x + c * 8.0f * scale;

            for (int row = 0; row < 8; ++row) {
                uint8_t bits = static_cast<uint8_t>(glyph[row]);
                if (bits == 0) continue;

                for (int col = 0; col < 8; ++col) {
                    if (bits & (1 << col)) {
                        glm::mat4 model(1.0f);
                        model = glm::translate(model, glm::vec3(baseX + col * scale, y + row * scale, 0.0f));
                        model = glm::scale(model, glm::vec3(scale, scale, 1.0f));
                        shader.setMat4("uModel", model);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                }
            }
        }
        glBindVertexArray(0);
    }
};
