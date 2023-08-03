#pragma once

#include "GameObject.h"
#include "Logger.h"
#include "GlobalConst.h"

#include <unordered_set>
class ObjectsPool
{
public:
    static GameObject *playerObject;
    static std::unordered_set<GameObject *> obstacles;
    static std::unordered_set<GameObject *> enemies;

    static std::unordered_set<GameObject *> bullets;

    ~ObjectsPool()
    {
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            GameObject *obj = *it;
            it = obstacles.erase(it);
            delete obj;
        }

        for (auto it = enemies.begin(); it != enemies.end(); ) {
            GameObject *obj = *it;
            it = enemies.erase(it);
            delete obj;
        }

        for (auto it = bullets.begin(); it != bullets.end(); ) {
            GameObject *obj = *it;
            it = bullets.erase(it);
            delete obj;
        }
    }

    static void kill(GameObject * obj) {
        {
            GameObject *agobj = dynamic_cast<GameObject *>(obj);
            auto it = bullets.find(agobj);
            if (it != bullets.end()) {
                bullets.erase(it);
                Logger::instance() << "Killing an object: " << agobj->name() << " " << obj->Id();
                delete agobj;
                return;
            }
        }
        {
            GameObject *agobj = dynamic_cast<GameObject *>(obj);
            auto it = enemies.find(agobj);
            if (it != enemies.end()) {
                enemies.erase(it);
                Logger::instance() << "Killing an object: " << agobj->name() << " " << agobj->Id();
                delete agobj;
                return;
            }
        }

        {
            auto it = obstacles.find(obj);
            if (it != obstacles.end()) {
                obstacles.erase(it);
                Logger::instance() << "Killing an object: " << obj->name() << " " << obj->Id();
                delete obj;
                return;
            }
        }
    }


    static GameObject *findNpcById(int id)
    {
        auto it = std::find_if(enemies.cbegin(), enemies.cend(), [id](GameObject *obj) { return obj->Id() == id; });
        return it != enemies.cend() ? *it : nullptr;
    }
};