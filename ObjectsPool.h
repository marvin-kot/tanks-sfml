#pragma once

#include "GameObject.h"

#include <unordered_set>
class ObjectsPool
{
public:
    static AnimatedGameObject *playerObject;
    static std::unordered_set<GameObject *> obstacles;
    static std::unordered_set<AnimatedGameObject *> enemies;

    ~ObjectsPool()
    {
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            GameObject *obj = *it;
            it = obstacles.erase(it);
            delete obj;
        }

        for (auto it = enemies.begin(); it != enemies.end(); ) {
            AnimatedGameObject *obj = *it;
            it = enemies.erase(it);
            delete obj;
        }
    }
};