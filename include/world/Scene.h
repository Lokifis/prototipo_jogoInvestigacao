#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "../graphics/Camera.h"
#include "../graphics/Shader.h"

using namespace std;

enum class ObjectType {
    Decoration,
    Clue,
    Door
};

struct SceneObject {
    string name;
    ObjectType  type = ObjectType::Decoration;
    string targetId;
    glm::vec3   position{0.0f};
    glm::vec3   size{1.0f};
    float       albedo = 0.5f;

    bool isInteractable() const { return type != ObjectType::Decoration; }
    bool isClue() const         { return type == ObjectType::Clue; }
    bool isDoor() const         { return type == ObjectType::Door; }
};

class Scene {
public:
    Scene();
    ~Scene();

    void loadRoom(const string& roomId);

    void addObject(const SceneObject& obj) { m_objects.push_back(obj); }
    void draw(const Shader& shader, const Camera& camera, int hoveredIndex) const;

    //Assets & JSON
    void setRoomObjects(const string& roomId, const vector<SceneObject>& objects) {
        m_currentRoom = roomId;
        m_objects = objects;
    }

    int pick(const Ray& ray) const;

    const vector<SceneObject>& objects() const     { return m_objects; }
    const string&              currentRoom() const { return m_currentRoom; }

private:
    vector<SceneObject> m_objects;
    string              m_currentRoom;
    unsigned int             m_vao = 0;
    unsigned int             m_vbo = 0;
};
