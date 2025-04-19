#pragma once

#include "GlobalTypes.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <sftools/Chronometer.hpp>

#include <random>
#include <vector>
#include <queue>
#include <utility>

class GameObject;


class MoveTrailBuffer
{
    std::queue<std::pair<int,int>> _buffer;
    int _fixedSize;

public:
    MoveTrailBuffer(int size) : _fixedSize(size) {}

    void push(int x, int y)
    {
        if (_buffer.size() >= _fixedSize)
            _buffer.pop();
        _buffer.push(std::make_pair(x, y));
    }

    std::pair<int,int> front(int onlyIfFull) const
    {
        if (_buffer.empty() || (onlyIfFull && _buffer.size() < _fixedSize))
            return std::make_pair(0, 0);
        return _buffer.front();
    }
};


class Controller
{

protected:
    GameObject *_gameObject;
    Controller *_trailerObjectController = nullptr;
    //sf::Clock _clock;
    sftools::Chronometer _clock;
    bool _pause = false;
    sf::Time _lastActionTime;
    bool _isMoving = false;

    int _currMoveX = 0;
    int _currMoveY = 0;
    int _moveSpeed;

    int _paralyzedForMs = 0;
    sftools::Chronometer _blinkClock;
    sftools::Chronometer _paralyzeClock;

    int _updatedOnFrame = 0;

    bool _addSpeed = false;
    int moveSpeedForCurrentFrame();
    void prepareMoveInDirection(globalTypes::Direction, int spd);
    void checkForGamePause();

    globalTypes::Direction directionBasedOnCurrentMovement() const;
public:
    Controller(GameObject *obj, int spd);
    virtual ~Controller() {}
    virtual void update() {}

    virtual void onDamaged() {};
    virtual void onCollided(GameObject *) {}
    virtual GameObject *onDestroyed() { return nullptr; }
    void paralyze(int msec);

    virtual bool previouslyMoved() const { return false; }
public:
    MoveTrailBuffer moveTrailBuffer;
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
    void onCollided(GameObject *) override;
    GameObject *onDestroyed() override;
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

class TrailerController : public Controller
{
    Controller *_ownerController;
public:
    TrailerController(GameObject *parent, Controller *owner) : Controller(parent, 0), _ownerController(owner) {}
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
    GameObject *onDestroyed() override;
};


class ExplosionController : public Controller
{
    bool _dontHurtParent;
public:
    ExplosionController(GameObject *parent, bool dontHurtParent);
    void update() override;

    static GameObject *createSmallExplosion(GameObject *parent);
    static GameObject *createBigExplosion(GameObject *parent, bool damaging);
    static std::vector<GameObject *> createHugeExplosion(GameObject *parent, int damage, bool damaging = false);
};


class LandmineController : public Controller
{
    bool _dontHurtParent;
public:
    LandmineController(GameObject *parent, bool dontHurtParent);
    void update() override;
    void onCollided(GameObject *) override;
    GameObject *onDestroyed() override;
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


class BlockageController : public Controller
{
public:
    BlockageController(GameObject *parent);
    void update() override;
    void onDamaged();
};


class JezekController : public Controller
{
    bool _dontHurtParent;
public:
    JezekController(GameObject *parent, bool dontHurtParent);
    void update() override;
    void onCollided(GameObject *) override;
};

class SkullController : public Controller
{
    int _timeoutMsec;
public:
    SkullController(GameObject *parent, int timeout);

    void update() override;
};

class CollectableXpController : public Controller
{
public:
    CollectableXpController(GameObject *parent);
    void onCollided(GameObject *) override;

};

class StaticCarController : public Controller
{
public:
    StaticCarController(GameObject *parent);
    void onCollided(GameObject *) override;
    GameObject *onDestroyed() override;
};


class PrizeBoxController : public Controller
{
public:
    PrizeBoxController(GameObject *parent);
    GameObject *onDestroyed() override;
};