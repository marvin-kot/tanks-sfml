
#include "GameObject.h"
#include "GlobalConst.h"
#include "Logger.h"
#include "ObjectsPool.h"

GameObject *ObjectsPool::playerObject = nullptr;
GameObject *ObjectsPool::eagleObject = nullptr;
GameObject *ObjectsPool::playerSpawnerObject = nullptr;
GameObject *ObjectsPool::bossObject = nullptr;
std::unordered_set<GameObject *> ObjectsPool::allGameObjects = {};
std::unordered_map<std::string, std::unordered_set<GameObject *>> ObjectsPool::objectsByType = {};
std::map<globalTypes::EagleWallDirection, GameObject *> ObjectsPool::eagleWalls = {};
std::unordered_map<int, net::ThinGameObject> ObjectsPool::thinGameObjects = {};

ObjectsPool::~ObjectsPool()
{
    clearEverything();
}

void ObjectsPool::clearEverything()
{
    for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ) {
        GameObject *obj = *it;
        it = allGameObjects.erase(it);
        if (obj->isFlagSet(GameObject::Player))
            ObjectsPool::playerObject = nullptr;
        if (obj->isFlagSet(GameObject::PlayerSpawner))
            ObjectsPool::playerSpawnerObject = nullptr;
        if (obj->isFlagSet(GameObject::Eagle))
            ObjectsPool::eagleObject = nullptr;

        auto& objByType = objectsByType.at(obj->type());
        auto it2 = objByType.find(obj);
        assert(it2 != objByType.end());

        objByType.erase(it2);
        auto dir = determineEagleWall(obj);
        if (dir != globalTypes::NotAnEagleWall)
            eagleWalls.erase(dir);

        delete obj;
    }
}

void ObjectsPool::kill(GameObject * obj)
{
    auto it = allGameObjects.find(obj);
    if (it != allGameObjects.end()) {
        allGameObjects.erase(it);
        Logger::instance() << "Killing an object: " << obj->type() << " " << obj->id() << "\n";

        auto& objByType = objectsByType.at(obj->type());
        auto it2 = objByType.find(obj);
        assert(it2 != objByType.end());

        objByType.erase(it2);

        auto dir = determineEagleWall(obj);
        if (dir != globalTypes::NotAnEagleWall)
            eagleWalls.erase(dir);

        delete obj;
        return;
    }
}

decltype(ObjectsPool::allGameObjects)::iterator ObjectsPool::kill(decltype(allGameObjects)::iterator it)
{
    assert(it != allGameObjects.end());

    GameObject *obj = *it;

    auto result = allGameObjects.erase(it);

    auto& objByType = objectsByType.at(obj->type());
    auto it2 = objByType.find(obj);
    assert(it2 != objByType.end());

    objByType.erase(it2);

    auto dir = determineEagleWall(obj);
    if (dir != globalTypes::NotAnEagleWall)
        eagleWalls.erase(dir);

    return result;
}



void ObjectsPool::addObject(GameObject *obj)
{
    assert(obj != nullptr);

    if (allGameObjects.find(obj) != allGameObjects.end()) {
        Logger::instance() << "object already exists. Ingoring\n";
        return;
    }

    allGameObjects.insert(obj);
    objectsByType[obj->type()].insert(obj);
}

void ObjectsPool::addEagleWall(globalTypes::EagleWallDirection dir, GameObject *obj)
{
    addObject(obj);
    eagleWalls[dir] = obj;
}

globalTypes::EagleWallDirection ObjectsPool::determineEagleWall(const GameObject * const obj)
{
    for (const auto w : eagleWalls) {
        if (w.second == obj)
            return w.first;
    }

    return globalTypes::NotAnEagleWall;
}

void ObjectsPool::iterateObjectAndCleanDeleted(std::function<void(GameObject *)> func)
{
    for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ) {
        GameObject *obj = *it;
        if (obj->mustBeDeleted()) {
            it = ObjectsPool::allGameObjects.erase(it);
            delete obj;
        } else {
            func(obj);
            ++it;
        }
    }
}

std::unordered_set<GameObject *>& ObjectsPool::getAllObjects()
{
    return allGameObjects;
}

std::unordered_set<GameObject *>& ObjectsPool::getObjectsByType(std::string type)
{
    return objectsByType[type];
}

std::unordered_set<GameObject *> ObjectsPool::getObjectsByTypes(std::vector<std::string> types)
{
    std::unordered_set<GameObject *> result;

    for (std::string type : types) {
        auto& objects = ObjectsPool::getObjectsByType(type);
        result.insert(objects.begin(), objects.end());
    }

    //Logger::instance() << "return " << result.size() << "objects";
    return result;
}



/*std::unordered_set<GameObject *> ObjectsPool::getObjectsByTypes(std::vector<std::string> types)
{
    std::unordered_set<GameObject *> result;

    for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ++it) {
        GameObject *obj = *it;
        bool found = false;
        for (std::string type : types) {
            if (obj->type() == type) {
                found = true;
                break;
            }
        }

        if (found)
            result.insert(obj);

    }

    return result;
}*/

int ObjectsPool::countObjectsByTypes(std::vector<std::string> types)
{
    int result = 0;

    for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ++it) {
        GameObject *obj = *it;
        for (std::string type : types) {
            if (obj->type() == type) {
                result++;
                break;
            }
        }
    }

    return result;
}


GameObject *ObjectsPool::findNpcById(int id)
{
    auto it = std::find_if(allGameObjects.cbegin(), allGameObjects.cend(), [id](GameObject *obj) { return obj->id() == id; });
    return it != allGameObjects.cend() ? *it : nullptr;
}

std::map<globalTypes::EagleWallDirection, GameObject *>& ObjectsPool::getEagleWalls()
{
    return eagleWalls;
}