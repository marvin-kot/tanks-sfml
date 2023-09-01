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
protected:
    int _actionTimeoutMs;
    sf::Clock _clock;
    GameObject *_gameObject;
    bool _piercing = false;
    int _damage;
    int _bulletSpeed;
    int _level;
public:
    Shootable(GameObject *parent, int level, int timeout, int bulletSpeed);
    inline void setActionTimeoutMs(int t) { _actionTimeoutMs = t; }
    inline int actionTimeoutMs() const { return _actionTimeoutMs; }
    inline void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    inline int bulletSpeed() const { return _bulletSpeed; }
    inline void setDamage(int d) { _damage = d; }
    inline int damage() const { return _damage; }
    void setPiercing(bool p) { _piercing = p; }
    inline void increaseLevel() { _level++; }
    inline void resetLevel() { _level = 0; }
    inline void setLevel(int val) { _level = val; }

    virtual bool shoot(globalTypes::Direction dir);

protected:
    virtual bool isShootingProhibited();

public:
    // default factories
    static Shootable *createDefaultPlayerShootable(GameObject *parent);
    static Shootable *createDefaultEnemyShootable(GameObject *parent);
    static Shootable *createDefaultRocketShootable(GameObject *parent);
    static Shootable *createDoubleRocketShootable(GameObject *parent);
};

struct FourDirectionShootable : public Shootable
{
    FourDirectionShootable(GameObject *parent, int bulletSpeed);
    bool shoot(globalTypes::Direction dir) override;

};


struct RocketShootable : public Shootable
{
    RocketShootable(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;
};

struct DoubleShootable : public Shootable
{
    DoubleShootable(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;
};

struct DoubleRocketShootable : public Shootable
{
    DoubleRocketShootable(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;
};

struct KamikazeShootable : public Shootable
{
    KamikazeShootable(GameObject *parent);
    bool shoot(globalTypes::Direction) override;
};

