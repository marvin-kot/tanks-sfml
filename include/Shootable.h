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
    int _bulletSpeed;
    int _damage;
public:
    Shootable(GameObject *parent) : parentObj(parent), _actionTimeoutMs(DefaultTimeoutMs),_bulletSpeed(DefaultBulletSpeed), _damage(DefaultDamage) {}
    void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    void setDamage(int d) { _damage = d; }
    bool shoot(globalTypes::Direction dir) {
        if (sf::milliseconds(_actionTimeoutMs) > _clock.getElapsedTime())
            return false;

        _clock.restart();

        Logger::instance() << "shoot";

        GameObject *bullet = new GameObject(parentObj, "bullet");
        bullet->setParentId(parentObj->id());
        bullet->setFlags(GameObject::Bullet);
        bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet));
        bullet->copyParentPosition(parentObj);

        // add to bullet pool
        ObjectsPool::addObject(bullet);


        return true;
    }


    static const int DefaultBulletSpeed = 8;
    static const int DoubleBulletSpeed = 16;
    static const int DefaultDamage = 1;
    static const int DoubleDamage = 2;
    static const int DefaultTimeoutMs = 1000;
    static const int HalvedTimeoutMs = 500;
};