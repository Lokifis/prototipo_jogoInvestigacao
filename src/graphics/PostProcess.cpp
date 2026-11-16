#include "../../include/graphics/PostProcess.h"

#include <algorithm>
#include <iostream>

PostProcess::PostProcess(int width, int height) : m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Textura de cor com filtro NEAREST (pixels nítidos ao ampliar)
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    // Profundidade
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    m_valid = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    if (!m_valid) std::cerr << "[PostProcess] Framebuffer incompleto" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &m_vao);
}

PostProcess::~PostProcess() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
    if (m_colorTex) glDeleteTextures(1, &m_colorTex);
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
}

PostProcess::Viewport PostProcess::computeViewport(int windowW, int windowH) const {
    // Escala inteira: cada pixel da imagem vira um quadrado de scale x scale
    const int scale = std::max(1, std::min(windowW / m_width, windowH / m_height));
    Viewport vp;
    vp.scale = scale;
    vp.w = m_width * scale;
    vp.h = m_height * scale;
    vp.x = (windowW - vp.w) / 2;
    vp.y = (windowH - vp.h) / 2;
    return vp;
}

void PostProcess::beginScene() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcess::present(const Shader& shader, int windowW, int windowH,
                          const glm::vec3& lightColor, const glm::vec3& darkColor) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    // Barras pretas ao redor da imagem
    glViewport(0, 0, windowW, windowH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const Viewport vp = computeViewport(windowW, windowH);
    glViewport(vp.x, vp.y, vp.w, vp.h);

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    shader.setInt("uScene", 0);
    shader.setVec2("uResolution", glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height)));
    shader.setVec3("uLightColor", lightColor);
    shader.setVec3("uDarkColor", darkColor);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

bool PostProcess::windowToScene(double x, double y, int windowW, int windowH, glm::vec2& out) const {
    const Viewport vp = computeViewport(windowW, windowH);
    const int topOffset = windowH - vp.h - vp.y;  // y do viewport no GL é contado de baixo

    const double sx = (x - vp.x) / vp.scale;
    const double sy = (y - topOffset) / vp.scale;
    if (sx < 0.0 || sy < 0.0 || sx >= m_width || sy >= m_height) return false;

    out = glm::vec2(static_cast<float>(sx), static_cast<float>(sy));
    return true;
}
