#include "../../include/world/Scene.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

namespace {
//colisão simples AABB
bool intersectAABB(const Ray& ray, const glm::vec3& mn, const glm::vec3& mx, float& tHit) {
    float tMin = 0.0f;
    float tMax = numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        const float o = ray.origin[i];
        const float d = ray.direction[i];
        if (fabs(d) < 1e-8f) {
            if (o < mn[i] || o > mx[i]) return false;
        } else {
            const float inv = 1.0f / d;
            float t1 = (mn[i] - o) * inv;\
            float t2 = (mx[i] - o) * inv;
            if (t1 > t2) swap(t1, t2);
            tMin = max(tMin, t1);
            tMax = min(tMax, t2);
            if (tMin > tMax) return false;
        }
    }
    tHit = tMin;
    return true;
}

}  // namespace

Scene::Scene() {
    vector<float> vertices;
    vertices.reserve(36 * 6);

    const float cu[4] = {-1.0f,  1.0f, 1.0f, -1.0f};
    const float cw[4] = {-1.0f, -1.0f, 1.0f,  1.0f};
    const int   idx[6] = {0, 1, 2, 0, 2, 3};

    for (int axis = 0; axis < 3; ++axis) {
        for (float sign : {-1.0f, 1.0f}) {
            glm::vec3 n(0.0f), u(0.0f), w(0.0f);
            n[axis] = sign;
            u[(axis + 1) % 3] = 1.0f;
            w[(axis + 2) % 3] = 1.0f;

            glm::vec3 corner[4];
            for (int i = 0; i < 4; ++i)
                corner[i] = 0.5f * n + 0.5f * cu[i] * u + 0.5f * cw[i] * w;

            for (int k : idx) {
                vertices.push_back(corner[k].x);
                vertices.push_back(corner[k].y);
                vertices.push_back(corner[k].z);
                vertices.push_back(n.x);
                vertices.push_back(n.y);
                vertices.push_back(n.z);
            }
        }
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Scene::~Scene() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Scene::loadRoom(const string& roomId) {
    m_objects.clear();
    m_currentRoom = roomId;

    if (roomId == "escritorio") {
        // Estrutura
        addObject({"Chão",              ObjectType::Decoration, "", { 0.0f, -0.1f,  0.0f}, {8.0f, 0.2f, 8.0f}, 0.55f});
        addObject({"Parede dos fundos", ObjectType::Decoration, "", { 0.0f,  1.5f, -4.1f}, {8.4f, 3.0f, 0.2f}, 0.70f});
        addObject({"Parede lateral",    ObjectType::Decoration, "", {-4.1f,  1.5f,  0.0f}, {0.2f, 3.0f, 8.0f}, 0.65f});

        // Móveis
        addObject({"Mesa",    ObjectType::Decoration, "", { 0.0f, 0.4f, -1.0f}, {2.4f, 0.8f, 1.2f}, 0.45f});
        addObject({"Armário", ObjectType::Decoration, "", {-3.0f, 0.9f, -3.0f}, {1.0f, 1.8f, 1.0f}, 0.35f});
        addObject({"Cadeira", ObjectType::Decoration, "", { 1.5f, 0.3f,  0.8f}, {0.6f, 0.6f, 0.6f}, 0.50f});

        // Porta de saída para o corredor
        addObject({"Porta para o Corredor", ObjectType::Door, "corredor", {3.9f, 1.1f, -1.5f}, {0.15f, 2.2f, 1.0f}, 0.30f});

        // Pistas do escritório
        addObject({"Carta rasgada",  ObjectType::Clue, "carta",   {-0.6f, 0.825f, -1.0f}, {0.5f, 0.05f, 0.35f}, 1.00f});
        addObject({"Relógio parado", ObjectType::Clue, "relogio", { 0.0f, 2.0f,   -3.95f}, {0.7f, 0.7f,  0.1f},  0.95f});
        addObject({"Copo quebrado",  ObjectType::Clue, "copo",    { 0.7f, 0.95f,  -1.0f}, {0.2f, 0.3f,  0.2f},  0.90f});
    }
    else if (roomId == "corredor") {
        // Estrutura mais estreita e longa
        addObject({"Chão",              ObjectType::Decoration, "", { 0.0f, -0.1f,  0.0f}, {4.0f, 0.2f, 10.0f}, 0.50f});
        addObject({"Parede esquerda",   ObjectType::Decoration, "", {-2.1f,  1.5f,  0.0f}, {0.2f, 3.0f, 10.0f}, 0.68f});
        addObject({"Parede dos fundos", ObjectType::Decoration, "", { 0.0f,  1.5f, -5.1f}, {4.4f, 3.0f,  0.2f}, 0.72f});

        // Portas conectando os cômodos
        addObject({"Voltar ao Escritório", ObjectType::Door, "escritorio", {-1.95f, 1.1f,  1.5f}, {0.15f, 2.2f, 1.0f}, 0.30f});
        addObject({"Porta da Cozinha",     ObjectType::Door, "cozinha",    { 0.0f,  1.1f, -4.95f}, {1.0f,  2.2f, 0.15f}, 0.30f});

        // Pista do corredor
        addObject({"Pegada de lama", ObjectType::Clue, "pegada", {0.2f, 0.01f, -1.5f}, {0.4f, 0.02f, 0.7f}, 0.20f});
    }
    else if (roomId == "cozinha") {
        // Estrutura
        addObject({"Chão",              ObjectType::Decoration, "", { 0.0f, -0.1f,  0.0f}, {6.0f, 0.2f, 6.0f}, 0.60f});
        addObject({"Parede dos fundos", ObjectType::Decoration, "", { 0.0f,  1.5f, -3.1f}, {6.4f, 3.0f, 0.2f}, 0.75f});
        addObject({"Parede lateral",    ObjectType::Decoration, "", {-3.1f,  1.5f,  0.0f}, {0.2f, 3.0f, 6.0f}, 0.70f});

        // Bancada
        addObject({"Bancada", ObjectType::Decoration, "", {-1.5f, 0.45f, -2.4f}, {2.5f, 0.9f, 0.8f}, 0.45f});

        // Porta de volta
        addObject({"Voltar ao Corredor", ObjectType::Door, "corredor", {2.9f, 1.1f, 0.0f}, {0.15f, 2.2f, 1.0f}, 0.30f});

        // Pista da cozinha
        addObject({"Frasco vazio", ObjectType::Clue, "frasco", {-1.2f, 1.0f, -2.4f}, {0.18f, 0.22f, 0.18f}, 0.95f});
    }
}

void Scene::draw(const Shader& shader, const Camera& camera, int hoveredIndex) const {
    shader.use();
    shader.setMat4("uView", camera.view());
    shader.setMat4("uProjection", camera.projection());
    shader.setVec3("uLightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));

    glBindVertexArray(m_vao);
    for (size_t i = 0; i < m_objects.size(); ++i) {
        const SceneObject& obj = m_objects[i];

        glm::mat4 model(1.0f);
        model = glm::translate(model, obj.position);
        model = glm::scale(model, obj.size);

        shader.setMat4("uModel", model);
        shader.setFloat("uAlbedo", obj.albedo);

        // Destaca tanto pistas quanto portas quando o mouse passa por cima
        const bool highlight = (static_cast<int>(i) == hoveredIndex) && obj.isInteractable();
        shader.setFloat("uHighlight", highlight ? 1.0f : 0.0f);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

int Scene::pick(const Ray& ray) const {
    int   best = -1;
    float bestT = numeric_limits<float>::max();

    for (size_t i = 0; i < m_objects.size(); ++i) {
        const SceneObject& obj = m_objects[i];
        const glm::vec3 half = obj.size * 0.5f;

        float t = 0.0f;
        if (intersectAABB(ray, obj.position - half, obj.position + half, t) && t < bestT) {
            bestT = t;
            best = static_cast<int>(i);
        }
    }

    // Retorna o objeto mais próximo se ele for uma pista OU uma porta
    if (best >= 0 && m_objects[best].isInteractable()) return best;
    return -1;
}
