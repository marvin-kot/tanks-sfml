#pragma once

#include "Controller.h"
#include "GlobalConst.h"
#include "SoundPlayer.h"
#include "ObjectsPool.h"
#include "Logger.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <sftools/Chronometer.hpp>

#include <string>

class GameObject;

class Shootable
{
protected:
    //sf::Clock _clock;
    sftools::Chronometer _shootClock;
    sftools::Chronometer _reloadClock;
    GameObject *_gameObject;
    bool _piercing = false;
    int _damage;
    int _bulletSpeed;
    int _level;

    int _maxBullets;
    int _bullets;
    int _shootTimeoutMs;
    int _reloadTimeoutMs;
    bool _instantReload = false;

    int _tempBullets;

    bool _pause = false;
    void checkForGamePause();
public:
    Shootable(GameObject *parent, int level, int timeout, int bulletSpeed, int maxBullets);
    inline void setReloadTimeoutMs(int t) { _reloadTimeoutMs = t; }
    inline int reloadTimeoutMs() const { return _reloadTimeoutMs; }
    inline void setShootTimeoutMs(int t) { _shootTimeoutMs = t; }
    inline int maxBullets() const { return _maxBullets; }
    inline int tempBullets() const { return _tempBullets; }
    void addTempBullets(int b);
    inline void setMaxBullets(int b) { _maxBullets = b; }
    inline int bullets() const { return _bullets; }
    inline void resetBullets() { _bullets = _maxBullets; }
    inline int shootTimeoutMs() const { return _shootTimeoutMs; }
    inline void setBulletSpeed(int bs) { _bulletSpeed = bs; }
    inline int bulletSpeed() const { return _bulletSpeed; }
    inline void setDamage(int d) { _damage = d; }
    inline void setInstantReload(bool v) {_instantReload = v;}
    inline int damage() const { return _damage; }
    void setPiercing(bool p) { _piercing = p; }
    inline void increaseLevel() { _level++; }
    inline void resetLevel() { _level = 0; }
    inline void setLevel(int val) { _level = val; }

    virtual bool shoot(globalTypes::Direction dir);

    virtual SoundPlayer::SoundType shootSound() const;

    void reloadByTimeout();

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
    FourDirectionShootable(GameObject *parent, int bulletSpeed, int maxBullets);
    bool shoot(globalTypes::Direction dir) override;

};


struct RocketShootable : public Shootable
{
    RocketShootable(GameObject *parent, int maxBullets);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;

    SoundPlayer::SoundType shootSound() const override;
};

struct DoubleShootable : public Shootable
{
    DoubleShootable(GameObject *parent, int maxBullets);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;
};

struct DoubleRocketShootable : public Shootable
{
    DoubleRocketShootable(GameObject *parent, int maxBullets);
    bool shoot(globalTypes::Direction dir) override;
    bool isShootingProhibited() override;
};

struct KamikazeShootable : public Shootable
{
    KamikazeShootable(GameObject *parent);
    bool shoot(globalTypes::Direction) override;
};

