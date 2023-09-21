#pragma once

#include "GlobalTypes.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <sftools/Chronometer.hpp>

#include <random>

class GameObject;

class Controller
{

protected:
    GameObject *_gameObject;
    //sf::Clock _clock;
    sftools::Chronometer _clock;
    bool _pause = false;
    sf::Time _lastActionTime;
    bool _isMoving = false;

    int _currMoveX = 0;
    int _currMoveY = 0;
    int _moveSpeed;

    bool _addSpeed = false;
    int moveSpeedForCurrentFrame();
    void prepareMoveInDirection(globalTypes::Direction, int spd);
    void checkForGamePause();
public:
    Controller(GameObject *obj, int spd);
    virtual ~Controller() {}
    virtual void update() {}

    virtual void onDamaged() {};
    virtual void onCollided(GameObject *) {}
};

class BulletController : public Controller
{
protected:
    int _damage;
    globalTypes::Direction _direction;
    bool _piercing;

public:
    BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg, bool pierce = false);
    void update() override;

    int speed() const { return _moveSpeed; }
    int damage() const { return _damage; }
    int loseDamage() { return --_damage; }
};

class RocketController : public BulletController
{
    int _startSpeed;
    int _currSpeed;
    int _maxSpeed;
public:
    RocketController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg);
    void update() override;
};

class SpawnController : public Controller
{
    enum SpawnStates {
        StartDelay,
        Starting,
        Waiting,
        PlayingAnimation,
        CreateObject
    };

    SpawnStates _state;
    const std::string _spawnableType;
    const sf::Time _spawnTimeout;
    const sf::Time _startSpawnDelay;

    sf::Clock _spawnAnimationclock;
    const sf::Time _spawnAnimationTime = sf::seconds(2);
    int _quantity;

    int _spawnBonusAtThisQuantity;

    static GameObject *createObject(std::string type);

public:
    SpawnController(GameObject *parent, std::string type, int delay, int timeout, int quantity);
    void update() override;

public:
    void setBonusSpawnWithProbability(int);
};

class PlayerSpawnController : public Controller
{
    enum SpawnStates {
        Starting,
        Spawning,
        Waiting,
        PlayingAnimation,
        CreateObject
    };

    SpawnStates _state;
    sf::Clock _spawnAnimationclock;
    const sf::Time _spawnAnimationTime = sf::seconds(1);
    int _initialPowerLevel;

    static GameObject *createObject();

public:
    PlayerSpawnController(GameObject *parent, int lives, int powerLevel);
    ~PlayerSpawnController();
    void update() override;
    void appendLife();
};


class ExplosionController : public Controller
{
    bool _dontHurtParent;
public:
    ExplosionController(GameObject *parent, bool dontHurtParent);
    void update() override;
};


class LandmineController : public Controller
{
    bool _dontHurtParent;
public:
    LandmineController(GameObject *parent, bool dontHurtParent);
    void update() override;
    void onCollided(GameObject *) override;
};

class StaticTurretController : public Controller
{
    globalTypes::Direction _direction;
    int _level;
public:
    StaticTurretController(GameObject *parent, globalTypes::Direction dir);
    void update() override;
    void onCollided(GameObject *) override;
    void setLevel(int l) { _level = l; }
    bool decideIfToShoot() const;
};