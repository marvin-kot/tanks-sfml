#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class GameObject;

class ObjectsPool
{
private:
    static std::unordered_set<GameObject *> allGameObjects;
    static std::unordered_map<std::string, std::unordered_set<GameObject *>> objectsByType;

public:
    static GameObject *playerObject;
    static GameObject *playerSpawnerObject;

    ~ObjectsPool();

    static void clearEverything();

    static void kill(GameObject * obj);
    static decltype(allGameObjects)::iterator kill(decltype(allGameObjects)::iterator);

    static void addObject(GameObject *obj);

    static void iterateObjectAndCleanDeleted(std::function<void(GameObject *)> func);

    static std::unordered_set<GameObject *> &getAllObjects();

    static std::unordered_set<GameObject *> &getObjectsByType(std::string type);

    static std::unordered_set<GameObject *> getObjectsByTypes(std::vector<std::string> types);
    static int countObjectsByTypes(std::vector<std::string> types);

    static GameObject *findNpcById(int id);
};