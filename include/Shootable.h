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
    sf::Clock _clock;
    int _actionTimeoutMs;
    int _bulletSpeed;
    int _damage;
protected:
    GameObject *_gameObject;
public:
    Shootable(GameObject *parent, int timeout)
    : _gameObject(parent)
    , _actionTimeoutMs(timeout)
    , _bulletSpeed(globalConst::DefaultBulletSpeed)
    , _damage(globalConst::DefaultDamage)
    {}

    void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    void setDamage(int d) { _damage = d; }
    bool shoot(globalTypes::Direction dir) {
        if (isShootingProhibited())
            return false;
        _clock.restart();
        Logger::instance() << "shoot";
        GameObject *bullet = new GameObject(_gameObject, "bullet");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet);
        bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet));
        bullet->copyParentPosition(_gameObject);

        // add to bullet pool
        ObjectsPool::addObject(bullet);


        return true;
    }

protected:
    virtual bool isShootingProhibited() {
        return sf::milliseconds(_actionTimeoutMs) > _clock.getElapsedTime();
    }

};

class PlayerShootable : public Shootable
{
    int _level;
public:
    PlayerShootable(GameObject *parent, int level) : Shootable(parent, globalConst::PlayerShootTimeoutMs), _level(level) {}
protected:
    bool isShootingProhibited() override {
        if (Shootable::isShootingProhibited()) return true;
        Logger::instance() << "player: check if shooting prohibited";
        auto bullets = ObjectsPool::getObjectsByType("bullet");
        Logger::instance() << "number of bullets:" << bullets.size();
        int countPlayerBullets = 0;

        for (auto b : bullets) {
            if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
                countPlayerBullets++;
        }

        return countPlayerBullets > _level;
    }

public:
    void increaseLevel() { _level++; }
    void resetLevel() {_level = 0; }
};