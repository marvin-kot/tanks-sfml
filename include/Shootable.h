#pragma once

#include "Controller.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Logger.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <string>

class GameObject;

class Shootable
{
    int _actionTimeoutMs;
protected:
    sf::Clock _clock;
    GameObject *_gameObject;
    bool _piercing = false;
    int _damage;
    int _bulletSpeed;
    int _level;
public:
    Shootable(GameObject *parent, int timeout, int bulletSpeed);
    inline void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    inline int actionTimeoutMs() const { return _actionTimeoutMs; }
    inline void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    inline int bulletSpeed() const { return _bulletSpeed; }
    inline void setDamage(int d) { _damage = d; }
    inline int damage() const { return _damage; }
    void setPiercing(bool p) { _piercing = p; }

    virtual bool shoot(globalTypes::Direction dir);

protected:
    virtual bool isShootingProhibited();
};

class PlayerShootable : public Shootable
{

public:
    PlayerShootable(GameObject *parent, int level);

    inline void increaseLevel() { _level++; }
    inline void resetLevel() { _level = 0; }
};

class EnemyTankShootable : public Shootable
{
public:
    EnemyTankShootable(GameObject *parent);
};


class FourDirectionShootable : public PlayerShootable
{
public:
    FourDirectionShootable(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;
};