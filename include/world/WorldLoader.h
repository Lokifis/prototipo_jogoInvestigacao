#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Scene.h"

// EXEMPLO DE HERANÇA #1 + EXCEÇÃO CUSTOMIZADA (Requisitos do T1)
class AssetLoadException : public std::runtime_error {
public:
    explicit AssetLoadException(const std::string& filePath, const std::string& reason)
        : std::runtime_error("Falha ao carregar asset '" + filePath + "': " + reason) {}
};

struct Clue {
    std::string id;
    std::string name;
    std::string description;
    bool        found = false;
};

struct Suspect {
    std::string id;
    std::string name;
    bool        isGuilty = false;
};

class WorldLoader {
public:
    static std::unordered_map<std::string, Clue> loadClues(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw AssetLoadException(filePath, "arquivo nao encontrado.");
        }

        std::unordered_map<std::string, Clue> clues;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string id, name, desc;
            if (!std::getline(ss, id, '|') ||
                !std::getline(ss, name, '|') ||
                !std::getline(ss, desc)) {
                throw AssetLoadException(filePath, "linha mal formatada -> " + line);
            }
            clues[id] = Clue{id, name, desc, false};
        }

        if (clues.empty()) {
            throw AssetLoadException(filePath, "nenhuma pista valida encontrada.");
        }
        return clues;
    }

    static std::unordered_map<std::string, std::vector<SceneObject>> loadRooms(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw AssetLoadException(filePath, "arquivo nao encontrado.");
        }

        std::unordered_map<std::string, std::vector<SceneObject>> rooms;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string roomId, typeStr, targetId, name, posStr, sizeStr, albedoStr;
            if (!std::getline(ss, roomId, '|') ||
                !std::getline(ss, typeStr, '|') ||
                !std::getline(ss, targetId, '|') ||
                !std::getline(ss, name, '|') ||
                !std::getline(ss, posStr, '|') ||
                !std::getline(ss, sizeStr, '|') ||
                !std::getline(ss, albedoStr)) {
                throw AssetLoadException(filePath, "linha mal formatada -> " + line);
            }

            SceneObject obj;
            obj.name     = name;
            obj.targetId = (targetId == "-") ? "" : targetId;
            obj.albedo   = std::stof(albedoStr); // Pode lançar std::invalid_argument

            if (typeStr == "Clue")      obj.type = ObjectType::Clue;
            else if (typeStr == "Door") obj.type = ObjectType::Door;
            else                        obj.type = ObjectType::Decoration;

            std::stringstream posStream(posStr);
            posStream >> obj.position.x >> obj.position.y >> obj.position.z;

            std::stringstream sizeStream(sizeStr);
            sizeStream >> obj.size.x >> obj.size.y >> obj.size.z;

            rooms[roomId].push_back(obj);
        }

        if (rooms.empty()) {
            throw AssetLoadException(filePath, "nenhuma sala valida encontrada.");
        }
        return rooms;
    }

    static std::vector<Suspect> loadSuspects(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw AssetLoadException(filePath, "arquivo nao encontrado.");
        }

        std::vector<Suspect> suspects;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string id, name, guiltyStr;
            if (!std::getline(ss, id, '|') ||
                !std::getline(ss, name, '|') ||
                !std::getline(ss, guiltyStr)) {
                throw AssetLoadException(filePath, "linha mal formatada -> " + line);
            }
            suspects.push_back(Suspect{id, name, guiltyStr == "1"});
        }

        if (suspects.empty()) {
            throw AssetLoadException(filePath, "nenhum suspeito cadastrado.");
        }
        return suspects;
    }
};
