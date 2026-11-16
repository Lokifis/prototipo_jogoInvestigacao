# Diagrama UML

Diagrama de classes em [Mermaid](https://mermaid.js.org/). Renderiza direto no GitHub
e no VS Code (extensão "Markdown Preview Mermaid Support").

```mermaid
classDiagram
    direction LR

    class GameState {
        <<enumeration>>
        Exploring
        Reading
        Notebook
        Accusation
        Ending
    }

    class ObjectType {
        <<enumeration>>
        Decoration
        Clue
        Door
    }

    class Clue {
        <<struct>>
        +string id
        +string name
        +string description
        +bool found
    }

    class Suspect {
        <<struct>>
        +string id
        +string name
        +bool isGuilty
    }

    class Game {
        <<main loop>>
        -unordered_map~string, Clue~ clueDatabase
        -unordered_map~string, vector~SceneObject~~ roomDatabase
        -vector~Suspect~ suspects
        -GameState state
        +runGame(window) int
    }

    class WorldLoader {
        <<utility>>
        +loadClues(filePath)$ unordered_map~string, Clue~
        +loadRooms(filePath)$ unordered_map~string, vector~SceneObject~~
    }

    class Shader {
        -unsigned int m_id
        +Shader(vertexPath, fragmentPath)
        +isValid() bool
        +use()
        +setInt(name, v)
        +setFloat(name, v)
        +setVec2(name, v)
        +setVec3(name, v)
        +setMat4(name, v)
    }

    class Ray {
        <<struct>>
        +vec3 origin
        +vec3 direction
    }

    class Camera {
        -vec3 m_target
        -float m_distance
        -float m_yaw
        -float m_pitch
        +setAspect(aspect)
        +rotate(dYaw, dPitch)
        +zoom(delta)
        +position() vec3
        +view() mat4
        +projection() mat4
        +screenPointToRay(x, y, w, h) Ray
    }

    class SceneObject {
        <<struct>>
        +string name
        +ObjectType type
        +string targetId
        +vec3 position
        +vec3 size
        +float albedo
        +isInteractable() bool
        +isClue() bool
        +isDoor() bool
    }

    class Scene {
        -vector~SceneObject~ m_objects
        -string m_currentRoom
        -unsigned int m_vao
        -unsigned int m_vbo
        +setRoomObjects(roomId, objects)
        +addObject(obj)
        +draw(shader, camera, hovered)
        +pick(ray) int
        +currentRoom() string
    }

    class PostProcess {
        -unsigned int m_fbo
        -unsigned int m_colorTex
        +beginScene()
        +present(shader, winW, winH, light, dark)
        +windowToScene(x, y, winW, winH, out) bool
    }

    class UIRenderer {
        -mat4 m_projection
        -unsigned int m_vao
        -unsigned int m_vbo
        +drawRect(shader, x, y, w, h, shade)
        +drawPanel(shader, x, y, w, h)
        +drawTextWrapped(shader, text, x, y, maxW, scale, shade)
        +drawButton(shader, label, x, y, w, h, mousePos, clicked) bool
    }

    Game --> Scene : possui
    Game --> Camera : possui
    Game --> PostProcess : possui
    Game --> UIRenderer : possui
    Game --> Shader : usa 3 (cena, ui e dithering)
    Game ..> WorldLoader : carrega dados
    Game o-- Clue : gerencia
    Game o-- Suspect : lista
    Game --> GameState

    Scene *-- SceneObject : contém
    SceneObject --> ObjectType
    Scene ..> Shader : usa no draw
    Scene ..> Camera : lê view/projection
    Scene ..> Ray : pick
    Camera ..> Ray : cria
    PostProcess ..> Shader : usa no present
    UIRenderer ..> Shader : usa no draw
    SceneObject ..> Clue : targetId referencia id (se Clue)
