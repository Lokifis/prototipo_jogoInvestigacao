# Jogo de Investigação

Jogo de investigação em **C++17 + OpenGL 3.3 (core)**, com estética 1-bit/dithering
inspirada em *Return of the Obra Dinn*.

> Status: protótipo jogável (exploração 3D, múltiplos cômodos, raycasting, FBO 1-bit, UI bitmap nativa, caderno e acusação).

## Como jogar

1. **Explorar:** Segure o botão direito do mouse e arraste para girar a câmera; use o scroll para zoom.
2. **Investigar e navegar:** Clique com o botão esquerdo sobre pistas para examiná-las ou sobre portas para mudar de cômodo.
3. **Caderno e acusação:** Pressione `N` (ou clique no botão superior) para revisar as pistas coletadas e fazer a acusação final.
4. **Atalhos:** `P` alterna entre as paletas de cores retrô; `F5` recarrega os dados das salas em tempo de execução; `ESC` fecha menus ou sai.

## Requisitos

- Compilador com suporte a C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- GPU/driver com OpenGL 3.3+
- GLFW 3.3+ e GLM (o CMake baixa automaticamente via `FetchContent` se não estiverem instalados no sistema)
- GLAD para OpenGL 3.3 Core em `include/glad`, `include/KHR` e `src/glad.c`

## Estrutura do projeto

```text
.
├── CMakeLists.txt
├── README.md
├── main.cpp
├── include/
│   ├── glad/ & KHR/
│   ├── graphics/       # Camera.h, PostProcess.h, Shader.h, UIRenderer.h, font8x8_basic.h
│   └── world/          # Scene.h, WorldLoader.h
├── src/                # glad.c e implementações .cpp (graphics/, world/)
├── shaders/            # scene.*, post.*, ui.* (copiados para o diretório de build)
├── assets/
│   └── data/           # clues.txt, rooms.txt (copiados para o diretório de build)
└── docs/
    └── UML.md
