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
    sf::Clock _clock;
    int _actionTimeoutMs;
    int _bulletSpeed;
    int _damage;
protected:
    GameObject *_gameObject;
public:
    Shootable(GameObject *parent, int timeout, int bulletSpeed);
    inline void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    inline void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    inline void setDamage(int d) { _damage = d; }
    bool shoot(globalTypes::Direction dir);

protected:
    virtual bool isShootingProhibited();
};

class PlayerShootable : public Shootable
{
    int _level;
public:
    PlayerShootable(GameObject *parent, int level);
protected:
    bool isShootingProhibited() override;

public:
    inline void increaseLevel() { _level++; }
    inline void resetLevel() { _level = 0; }
};

class EnemyTankShootable : public Shootable
{
    int _level;
public:
    EnemyTankShootable(GameObject *parent);
protected:
    bool isShootingProhibited() override;
};