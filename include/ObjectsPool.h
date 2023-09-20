#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>

#include "GlobalTypes.h"

#include "NetGameTypes.h"

class GameObject;

class ObjectsPool
{
private:
    static std::unordered_set<GameObject *> allGameObjects;
    static std::unordered_map<std::string, std::unordered_set<GameObject *>> objectsByType;
    static std::map<globalTypes::EagleWallDirection, GameObject *> eagleWalls;
    static std::unordered_map<int, std::unordered_set<GameObject *> > objectsByDrawOrder;


    static void cleanupOtherContainers(GameObject *);
public:
    static std::queue<GameObject *> objectsToDelete;

    static std::unordered_map<int, net::ThinGameObject> thinGameObjects;

    static GameObject *playerObject;
    static GameObject *eagleObject;
    static GameObject *playerSpawnerObject;
    static GameObject *bossObject;

    ~ObjectsPool();

    static void clearEverything();

    static void kill(GameObject * obj);
    static decltype(allGameObjects)::iterator kill(decltype(allGameObjects)::iterator);

    static void addObject(GameObject *obj);
    static void addEagleWall(globalTypes::EagleWallDirection, GameObject *);
    static globalTypes::EagleWallDirection determineEagleWall(const GameObject * const obj);

    static void iterateObjectAndCleanDeleted(std::function<void(GameObject *)> func);

    static std::unordered_set<GameObject *> &getAllObjects();

    static std::unordered_set<GameObject *> &getObjectsByType(std::string type);

    static std::unordered_set<GameObject *> getObjectsByTypes(std::vector<std::string> types);
    static int countObjectsByTypes(std::vector<std::string> types);

    static std::unordered_set<GameObject *> &getObjectsByDrawOrder(int order);

    static GameObject *findNpcById(int id);

    static std::map<globalTypes::EagleWallDirection, GameObject *> &getEagleWalls();
};