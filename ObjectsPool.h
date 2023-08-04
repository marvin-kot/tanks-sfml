#pragma once

#include "GameObject.h"
#include "Logger.h"
#include "GlobalConst.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
class ObjectsPool
{
private:
    static std::unordered_set<GameObject *> allGameObjects;
    static std::unordered_map<std::string, std::unordered_set<GameObject *>> objectsByType;

public:
    static GameObject *playerObject;

    ~ObjectsPool()
    {
        clearEverything();
    }

    static void clearEverything()
    {
        for (auto it = allGameObjects.begin(); it != allGameObjects.end(); ) {
            GameObject *obj = *it;
            it = allGameObjects.erase(it);
            delete obj;
        }
    }

    static void kill(GameObject * obj) {
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

    static void addObject(GameObject *obj)
    {
        assert(obj != nullptr);

        if (allGameObjects.find(obj) != allGameObjects.end()) {
            Logger::instance() << "object already exists. Ingoring";
            return;
        }

        allGameObjects.insert(obj);
        objectsByType[obj->type()].insert(obj);
    }

    void iterateObjectAndCleanDeleted(std::function<void(GameObject *)> func)
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

    static std::unordered_set<GameObject *> &getAllObjects()
    {
        return allGameObjects;
    }

    static std::unordered_set<GameObject *> &getObjectsByType(std::string type)
    {
        return objectsByType.at(type);
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
    static std::unordered_set<GameObject *> getObjectsByTypes(std::vector<std::string> types)
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


    static GameObject *findNpcById(int id)
    {
        auto it = std::find_if(allGameObjects.cbegin(), allGameObjects.cend(), [id](GameObject *obj) { return obj->id() == id; });
        return it != allGameObjects.cend() ? *it : nullptr;
    }
};