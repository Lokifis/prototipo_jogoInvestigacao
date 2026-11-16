/*
 * ============================================================================
 * JOGO DE INVESTIGAÇÃO - Programação Orientada a Objetos (PUCRS)
 * ============================================================================
 * Direção de arte inspirada em "Return of the Obra Dinn" (Lucas Pope).
 *
 * Bibliotecas e Repositórios de Referência (Open-Source):
 * - GLFW (Janela, Contexto e Input) : https://github.com/glfw/glfw
 * - GLM  (Matemática 3D / Matrizes)  : https://github.com/g-truc/glm
 * - GLAD (Loader OpenGL 3.3 Core)    : https://github.com/Dav1dde/glad
 * - font8x8 (Fonte Bitmap 8x8)       : https://github.com/dhepper/font8x8
 * ============================================================================
 */

#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/graphics/Camera.h"
#include "include/graphics/PostProcess.h"
#include "include/graphics/Shader.h"
#include "include/graphics/UIRenderer.h"
#include "include/world/Scene.h"
#include "include/world/WorldLoader.h"

using namespace std;

// ============================================================================
// CONFIGURAÇÕES GLOBAIS E ESTADOS
// ============================================================================

const unsigned int SCR_WIDTH     = 1280;
const unsigned int SCR_HEIGHT    = 720;
const int          RENDER_WIDTH  = 640;
const int          RENDER_HEIGHT = 360;

enum class GameState {
    Exploring,   // Movendo a câmera e procurando pistas/portas
    Reading,     // Lendo o detalhe de uma pista recém-clicada
    Notebook,    // Revisando todas as pistas coletadas
    Accusation,  // Escolhendo o culpado
    Ending       // Tela de desfecho
};

struct Palette {
    const char* name;
    glm::vec3   light;
    glm::vec3   dark;
};

// ============================================================================
// UTILITÁRIOS
// ============================================================================

static double g_scrollY = 0.0;

void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    g_scrollY += yoffset;
}

struct InputHelper {
    unordered_map<int, bool> prevStates;

    bool isKeyJustPressed(GLFWwindow* window, int key) {
        bool current = glfwGetKey(window, key) == GLFW_PRESS;
        bool justPressed = current && !prevStates[key];
        prevStates[key] = current;
        return justPressed;
    }

    bool isMouseJustPressed(GLFWwindow* window, int button) {
        int id = -(button + 1);
        bool current = glfwGetMouseButton(window, button) == GLFW_PRESS;
        bool justPressed = current && !prevStates[id];
        prevStates[id] = current;
        return justPressed;
    }
};

static string assetPath(const string& relative) {
    if (filesystem::exists(relative)) return relative;
#ifdef ASSET_DIR
    return string(ASSET_DIR) + relative;
#else
    return relative;
#endif
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================

static int runGame(GLFWwindow* window) {
    // 1. Shaders e Pós-Processamento
    Shader sceneShader(assetPath("shaders/scene.vert"), assetPath("shaders/scene.frag"));
    Shader postShader(assetPath("shaders/post.vert"), assetPath("shaders/post.frag"));
    Shader uiShader(assetPath("shaders/ui.vert"), assetPath("shaders/ui.frag"));

    if (!sceneShader.isValid() || !postShader.isValid() || !uiShader.isValid()) {
        cerr << "[Erro] Falha ao carregar os shaders (verifique scene, post e ui)." << endl;
        return -1;
    }

    PostProcess post(RENDER_WIDTH, RENDER_HEIGHT);
    if (!post.isValid()) return -1;

    UIRenderer ui(RENDER_WIDTH, RENDER_HEIGHT);
    Camera camera;
    camera.setAspect(static_cast<float>(RENDER_WIDTH) / static_cast<float>(RENDER_HEIGHT));

    // 2. Carregamento Externo dos Dados (com Exceção Customizada)
    unordered_map<string, Clue>                clueDatabase;
    unordered_map<string, vector<SceneObject>> roomDatabase;
    vector<Suspect>                            suspects;

    try {
        clueDatabase = WorldLoader::loadClues(assetPath("assets/data/clues.txt"));
        roomDatabase = WorldLoader::loadRooms(assetPath("assets/data/rooms.txt"));
        suspects     = WorldLoader::loadSuspects(assetPath("assets/data/suspects.txt"));
    } catch (const AssetLoadException& e) {
        cerr << "[ERRO DE ASSET] " << e.what() << endl;
        return -1;
    } catch (const exception& e) {
        cerr << "[ERRO INESPERADO] " << e.what() << endl;
        return -1;
    }

    // Função Lambda (requisito do T1): contabiliza pistas já encontradas
    auto countFoundClues = [&clueDatabase]() -> int {
        return static_cast<int>(count_if(clueDatabase.begin(), clueDatabase.end(),
            [](const auto& entry) { return entry.second.found; }));
    };

    Scene scene;
    const string initialRoom = roomDatabase.count("escritorio") ? "escritorio" : roomDatabase.begin()->first;
    scene.setRoomObjects(initialRoom, roomDatabase[initialRoom]);

    const vector<Palette> palettes = {
        {"Papel",          glm::vec3(0.93f, 0.90f, 0.78f), glm::vec3(0.08f, 0.08f, 0.10f)},
        {"Preto e branco", glm::vec3(1.00f, 1.00f, 1.00f), glm::vec3(0.00f, 0.00f, 0.00f)},
        {"Fosforo verde",  glm::vec3(0.55f, 0.95f, 0.55f), glm::vec3(0.02f, 0.08f, 0.03f)},
        {"Ambar",          glm::vec3(1.00f, 0.69f, 0.20f), glm::vec3(0.10f, 0.05f, 0.00f)},
    };
    size_t paletteIndex = 0;

    // 3. Variáveis de Estado do Jogo
    GameState   state         = GameState::Exploring;
    const Clue* activeClue    = nullptr;
    string      endingMessage = "";

    InputHelper input;
    bool        dragging    = false;
    double      lastX = 0.0, lastY = 0.0;
    int         lastHovered = -2;

    // 4. Game Loop
    while (!glfwWindowShouldClose(window)) {
        int fbW, fbH, winW, winH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glfwGetWindowSize(window, &winW, &winH);
        if (fbW == 0 || fbH == 0 || winW == 0 || winH == 0) {
            glfwPollEvents();
            continue;
        }

        // Captura posição do mouse e converte para a resolução interna (640x360)
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        const double fx = mx * fbW / winW;
        const double fy = my * fbH / winH;
        glm::vec2 scenePos(-1.0f);
        bool mouseInsideScene = post.windowToScene(fx, fy, fbW, fbH, scenePos);

        const bool leftJustClicked = input.isMouseJustPressed(window, GLFW_MOUSE_BUTTON_LEFT);

        // --- ATALHOS GLOBAIS ---
        if (input.isKeyJustPressed(window, GLFW_KEY_P)) {
            paletteIndex = (paletteIndex + 1) % palettes.size();
        }

        if (input.isKeyJustPressed(window, GLFW_KEY_F5)) {
            try {
                roomDatabase = WorldLoader::loadRooms(assetPath("assets/data/rooms.txt"));
                if (roomDatabase.count(scene.currentRoom())) {
                    scene.setRoomObjects(scene.currentRoom(), roomDatabase[scene.currentRoom()]);
                    cout << "[Dev] Sala '" << scene.currentRoom() << "' recarregada." << endl;
                }
            } catch (const AssetLoadException& e) {
                cerr << "[Dev - Falha no Hot-Reload] " << e.what() << endl;
            }
        }

        if (input.isKeyJustPressed(window, GLFW_KEY_ESCAPE)) {
            if (state == GameState::Reading || state == GameState::Notebook || state == GameState::Accusation) {
                state = GameState::Exploring;
            } else {
                glfwSetWindowShouldClose(window, true);
            }
        }

        if (input.isKeyJustPressed(window, GLFW_KEY_N) && state != GameState::Ending) {
            state = (state == GameState::Notebook) ? GameState::Exploring : GameState::Notebook;
        }

        // --- LÓGICA DO MODO EXPLORAÇÃO ---
        int hovered = -1;

        if (state == GameState::Exploring) {
            const bool rightMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightMouse && dragging) {
                camera.rotate(static_cast<float>(-(mx - lastX)) * 0.3f,
                              static_cast<float>(my - lastY) * 0.3f);
            }
            dragging = rightMouse;
            lastX = mx;
            lastY = my;

            if (g_scrollY != 0.0) {
                camera.zoom(static_cast<float>(g_scrollY) * 0.5f);
                g_scrollY = 0.0;
            }

            if (!dragging && mouseInsideScene) {
                Ray ray = camera.screenPointToRay(scenePos.x, scenePos.y, RENDER_WIDTH, RENDER_HEIGHT);
                hovered = scene.pick(ray);
            }

            if (leftJustClicked && hovered >= 0) {
                const SceneObject& obj = scene.objects()[hovered];

                if (obj.isClue()) {
                    auto it = clueDatabase.find(obj.targetId);
                    if (it != clueDatabase.end()) {
                        it->second.found = true;
                        activeClue = &it->second;
                        state = GameState::Reading;
                    }
                } else if (obj.isDoor()) {
                    if (roomDatabase.count(obj.targetId)) {
                        scene.setRoomObjects(obj.targetId, roomDatabase[obj.targetId]);
                        lastHovered = -2;
                    }
                }
            }
        }

        if (hovered != lastHovered) {
            string title = "Investigacao [" + scene.currentRoom() + "]";
            if (hovered >= 0 && state == GameState::Exploring) {
                title += " - " + scene.objects()[hovered].name;
            }
            glfwSetWindowTitle(window, title.c_str());
            lastHovered = hovered;
        }

        // --- RENDER (UI NO FBO 640x360) ---
        post.beginScene();
        scene.draw(sceneShader, camera, hovered);

        glDisable(GL_DEPTH_TEST);

        const int collectedCount = countFoundClues();

        // 1. HUD Superior
        if (state != GameState::Ending) {
            ui.drawTextWrapped(uiShader, "LOCAL: " + scene.currentRoom(), 10, 10, 300);

            string btnLabel = "[N] CADERNO (" + to_string(collectedCount) + "/" + to_string(clueDatabase.size()) + ")";
            if (ui.drawButton(uiShader, btnLabel, 470, 6, 160, 20, scenePos, leftJustClicked)) {
                state = (state == GameState::Notebook) ? GameState::Exploring : GameState::Notebook;
            }
        }

        // 2. Barra inferior com nome do objeto sob a mira
        if (hovered >= 0 && state == GameState::Exploring) {
            const SceneObject& obj = scene.objects()[hovered];
            string prefix = obj.isDoor() ? "IR PARA: " : "EXAMINAR: ";
            ui.drawPanel(uiShader, 140, 322, 360, 28);
            ui.drawTextWrapped(uiShader, prefix + obj.name, 152, 332, 336);
        }

        // 3. Janela Modal de Leitura de Pista
        if (state == GameState::Reading && activeClue != nullptr) {
            ui.drawPanel(uiShader, 100, 85, 440, 190);
            ui.drawTextWrapped(uiShader, activeClue->name, 118, 102, 404, 1.5f);
            ui.drawRect(uiShader, 118, 122, 404, 1, 1.0f);
            ui.drawTextWrapped(uiShader, activeClue->description, 118, 134, 404, 1.0f);

            if (ui.drawButton(uiShader, "CONTINUAR [ESC]", 395, 242, 128, 20, scenePos, leftJustClicked)) {
                state = GameState::Exploring;
            }
        }

        // 4. Tela do Caderno de Pistas
        if (state == GameState::Notebook) {
            ui.drawPanel(uiShader, 50, 34, 540, 296);
            ui.drawTextWrapped(uiShader, "CADERNO DE INVESTIGACAO", 68, 50, 500, 1.5f);
            ui.drawRect(uiShader, 68, 70, 504, 1, 1.0f);

            float listY = 84.0f;
            for (const auto& [id, clue] : clueDatabase) {
                if (!clue.found) continue;
                ui.drawTextWrapped(uiShader, "* " + clue.name + ": " + clue.description, 68, listY, 504, 1.0f);
                listY += 32.0f;
            }

            if (collectedCount == 0) {
                ui.drawTextWrapped(uiShader, "Nenhuma pista coletada ainda. Clique nos objetos da cena.", 68, 88, 504);
            }

            if (ui.drawButton(uiShader, "ACUSAR CULPADO", 68, 296, 136, 20, scenePos, leftJustClicked)) {
                state = GameState::Accusation;
            }
            if (ui.drawButton(uiShader, "FECHAR", 498, 296, 74, 20, scenePos, leftJustClicked)) {
                state = GameState::Exploring;
            }
        }

        // 5. Tela de Acusação
        if (state == GameState::Accusation) {
            ui.drawPanel(uiShader, 120, 60, 400, 240);
            ui.drawTextWrapped(uiShader, "QUEM COMETEU O CRIME?", 140, 78, 360, 1.5f);
            ui.drawRect(uiShader, 140, 98, 360, 1, 1.0f);

            float btnY = 115.0f;
            for (const auto& suspect : suspects) {
                if (ui.drawButton(uiShader, suspect.name, 140, btnY, 360, 24, scenePos, leftJustClicked)) {
                    if (suspect.isGuilty) {
                        endingMessage = "CASO ENCERRADO! Voce deduziu corretamente que " + suspect.name + " era o culpado.";
                    } else {
                        endingMessage = "ACUSACAO ERRADA! " + suspect.name + " era inocente e o verdadeiro culpado escapou.";
                    }
                    state = GameState::Ending;
                }
                btnY += 34.0f;
            }

            if (ui.drawButton(uiShader, "CANCELAR", 418, 266, 82, 20, scenePos, leftJustClicked)) {
                state = GameState::Notebook;
            }
        }

        // 6. Tela Final (Desfecho)
        if (state == GameState::Ending) {
            ui.drawPanel(uiShader, 100, 100, 440, 160);
            ui.drawTextWrapped(uiShader, "RELATORIO FINAL", 120, 118, 400, 1.5f);
            ui.drawRect(uiShader, 120, 138, 400, 1, 1.0f);
            ui.drawTextWrapped(uiShader, endingMessage, 120, 152, 400, 1.0f);

            if (ui.drawButton(uiShader, "SAIR DO JOGO", 390, 226, 130, 20, scenePos, leftJustClicked)) {
                glfwSetWindowShouldClose(window, true);
            }
        }

        glEnable(GL_DEPTH_TEST);

        post.present(postShader, fbW, fbH, palettes[paletteIndex].light, palettes[paletteIndex].dark);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

// ============================================================================
// ENTRADA
// ============================================================================

int main() {
    if (!glfwInit()) {
        cerr << "Falha ao inicializar o GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Jogo de Investigação", nullptr, nullptr);
    if (!window) {
        cerr << "Falha ao criar a janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Falha ao inicializar o GLAD" << endl;
        glfwTerminate();
        return -1;
    }

    const int result = runGame(window);

    glfwTerminate();
    return result == 0 ? 0 : -1;
}
