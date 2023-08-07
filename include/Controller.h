#pragma once

#include "GlobalConst.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <random>


class GameObject;

class Controller
{

protected:
    GameObject *_gameObject;
    sf::Clock _clock;
    sf::Time _lastActionTime;
    bool _isMoving = false;
public:
    Controller(GameObject *obj);
    virtual void update() {}
};

class StupidController : public Controller
{
    const int moveSpeed = 3;
    const sf::Time actionTimeout = sf::seconds(0.5);

    int currMoveX = 0;
    int currMoveY = 0;

    std::uniform_int_distribution<int> distribution;
public:
    StupidController(GameObject *obj);
    void update() override;
};

class PlayerController : public Controller
{
    const int moveSpeed = 5;
public:
    PlayerController(GameObject *obj);
    void update() override;
};



class BulletController : public Controller
{
    const int moveSpeed = 10;
    globalTypes::Direction _direction;

public:
    BulletController(GameObject *obj, globalTypes::Direction dir);
    void update() override;
};

class SpawnController : public Controller
{
    enum SpawnStates {
        Starting,
        Waiting,
        PlayingAnimation,
        CreateObject
    };

    SpawnStates _state = PlayingAnimation;
    const std::string _spawnableType;
    const sf::Time _spawnTimeout;

    sf::Clock _spawnAnimationclock;
    const sf::Time _spawnAnimationTime = sf::seconds(2);
    int _quantity;

    static GameObject *createObject(std::string type);

public:
    SpawnController(GameObject *parent, std::string type, int timeout, int quantity);
    void update() override;
};