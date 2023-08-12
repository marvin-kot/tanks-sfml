#include "ObjectsPool.h"

GameObject *ObjectsPool::playerObject = nullptr;
std::unordered_set<GameObject *> ObjectsPool::allGameObjects = {};
std::unordered_map<std::string, std::unordered_set<GameObject *>> ObjectsPool::objectsByType = {};

ObjectsPool::~ObjectsPool()
{
    clearEverything();
}

void ObjectsPool::clearEverything()
{
    for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ) {
        GameObject *obj = *it;
        it = allGameObjects.erase(it);
        delete obj;
    }
}

void ObjectsPool::kill(GameObject * obj)
{
    auto it = allGameObjects.find(obj);
    if (it != allGameObjects.end()) {
        allGameObjects.erase(it);
        Logger::instance() << "Killing an object: " << obj->type() << " " << obj->id();

        auto& objByType = objectsByType.at(obj->type());
        auto it2 = objByType.find(obj);
        assert(it2 != objByType.end());

        objByType.erase(it2);

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

    return result;
}

void ObjectsPool::addObject(GameObject *obj)
{
    assert(obj != nullptr);

    if (allGameObjects.find(obj) != allGameObjects.end()) {
        Logger::instance() << "object already exists. Ingoring";
        return;
    }

    allGameObjects.insert(obj);
    objectsByType[obj->type()].insert(obj);
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

/*    static std::unordered_set<GameObject *> getObjectsByTypes(std::vector<std::string> types)
{
    std::unordered_set<GameObject *> result;

    for (std::string type : types) {
        auto& objects = ObjectsPool::getObjectsByType(type);
        result.insert(objects.begin(), objects.end());
    }

    Logger::instance() << "return " << result.size() << "objects";
    return result;
}
*/


std::unordered_set<GameObject *> ObjectsPool::getObjectsByTypes(std::vector<std::string> types)
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
}

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