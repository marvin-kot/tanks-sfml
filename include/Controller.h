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
};

class TankRandomController : public Controller
{
    const sf::Time _actionTimeout;

    std::uniform_int_distribution<int> distribution;
    int trySqueeze();
public:
    TankRandomController(GameObject *parent, int spd, float timeoutSec);
    void update() override;
};




class BulletController : public Controller
{
    const int _damage;
    globalTypes::Direction _direction;

public:
    BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg);
    void update() override;

    int speed() const { return _moveSpeed; }
    int damage() const { return _damage; }
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
        Waiting,
        PlayingAnimation,
        CreateObject
    };

    SpawnStates _state;
    sf::Clock _spawnAnimationclock;
    const sf::Time _spawnAnimationTime = sf::seconds(1);
    int _lives;
    int _initialPowerLevel;

    static GameObject *createObject();

public:
    PlayerSpawnController(GameObject *parent, int lives, int powerLevel);
    ~PlayerSpawnController();
    void update() override;
    void appendLife();
};
