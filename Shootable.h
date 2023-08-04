#pragma once


#include "GameObject.h"
#include "Controller.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Logger.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <string>


class Shootable
{
    GameObject *parentObj;
    sf::Clock _clock;
    int _actionTimeoutMs = 1000;
public:
    Shootable(GameObject *parent) : parentObj(parent) {}
    void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    bool shoot(globalTypes::Direction dir) {
        if (sf::milliseconds(_actionTimeoutMs) > _clock.getElapsedTime())
            return false;

        _clock.restart();

        Logger::instance() << "shoot";

        GameObject *bullet = new GameObject("bullet");
        bullet->setParentId(parentObj->id());
        bullet->setFlags(GameObject::Bullet);
        bullet->setController(new BulletController(bullet, dir));
        bullet->createSpriteRenderer();
        bullet->copyParentPosition(parentObj);

        // add to bullet pool
        ObjectsPool::addObject(bullet);


        return true;
    }
};