#include "Controller.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"


Controller::Controller(GameObject *obj)
: _gameObject(obj)
{}

StupidController::StupidController(GameObject *obj)
: Controller(obj), distribution(0, 4)
{}

void StupidController::update()
{
    if (_clock.getElapsedTime() > actionTimeout) {
        _clock.restart();
        // change decision
        int dir = distribution(Utils::generator);
        switch (dir) {
            case 0: // stay / shoot
                currMoveX = currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->shoot();
                break;
            case 1: // left
                currMoveX = -moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Left);
                _gameObject->setCurrentAnimation("left");
                _isMoving = true;
                break;
            case 2: // up
                currMoveY = -moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Up);
                _gameObject->setCurrentAnimation("up");
                _isMoving = true;
                break;
            case 3: // right
                currMoveX = moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Right);
                _gameObject->setCurrentAnimation("right");
                _isMoving = true;
                break;
            case 4: // down
                currMoveY = moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Down);
                _gameObject->setCurrentAnimation("down");
                _isMoving = true;
                break;
        }
    }

    _gameObject->move(currMoveX, currMoveY);
}

PlayerController::PlayerController(GameObject *obj)
: Controller(obj)
{}

void PlayerController::update()
{
    bool action = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        _isMoving = false;
        if (_gameObject->shoot())
            SoundPlayer::instance().playShootSound();;
    } else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        _gameObject->move(-moveSpeed, 0);
        _gameObject->setCurrentDirection(globalTypes::Left);
        _gameObject->setCurrentAnimation("left");
        _isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        _gameObject->move(0, -moveSpeed);
        _gameObject->setCurrentDirection(globalTypes::Up);
        _gameObject->setCurrentAnimation("up");
        _isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        _gameObject->move(moveSpeed, 0);
        _gameObject->setCurrentDirection(globalTypes::Right);
        _gameObject->setCurrentAnimation("right");
        _isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        _gameObject->move(0, moveSpeed);
        _gameObject->setCurrentDirection(globalTypes::Down);
        _gameObject->setCurrentAnimation("down");
        _isMoving = true;
    } else
        _isMoving = false;

    if (_isMoving) {
        _lastActionTime = _clock.getElapsedTime();
        SoundPlayer::instance().playTankMoveSound();
        _gameObject->restartAnimation();
    } else {
        SoundPlayer::instance().playTankStandSound();
        _gameObject->stopAnimation();
    }
}


BulletController::BulletController(GameObject *obj, globalTypes::Direction dir)
: Controller(obj), _direction(dir)
{}

void BulletController::update()
{
    if (_direction == globalTypes::Left)
    {
        _gameObject->move(-moveSpeed, 0);
        _gameObject->setCurrentAnimation("left");
    } else if (_direction == globalTypes::Up) {
        _gameObject->move(0, -moveSpeed);
        _gameObject->setCurrentAnimation("up");
    } else if (_direction == globalTypes::Right) {
        _gameObject->move(moveSpeed, 0);
        _gameObject->setCurrentAnimation("right");
    } else if (_direction == globalTypes::Down) {
        _gameObject->move(0, moveSpeed);
        _gameObject->setCurrentAnimation("down");
    }
}

///// Spawner
SpawnController::SpawnController(GameObject *parent, std::string type, int timeout, int quantity)
: Controller(parent), _spawnableType(type), _spawnTimeout(sf::seconds(timeout)), _quantity(quantity)
{
    _gameObject->hide(true);
    _state = Starting;
}


void SpawnController::update()
{
    if (_quantity < 1) {
        _gameObject->_deleteme = true;
        return;
    }

    switch (_state) {
        case Waiting:
            if (_clock.getElapsedTime() > _spawnTimeout)
                _state = Starting;
            break;
        case Starting:
            Logger::instance() << "Starting";
            _gameObject->hide(false);
            _spawnAnimationclock.restart();
            _state = PlayingAnimation;
            break;
        case PlayingAnimation:
            if (_spawnAnimationclock.getElapsedTime() > _spawnAnimationTime)
                _state = CreateObject;
            break;
        case CreateObject:
            {
                GameObject * newNpc = createObject(_spawnableType);

                if (newNpc) {
                    newNpc->copyParentPosition(_gameObject);
                    ObjectsPool::addObject(newNpc);
                    _quantity--;
                }

                _clock.restart();
                _gameObject->hide(true);
                _state = Waiting;
            }
            break;
    }
}

GameObject *SpawnController::createObject(std::string type)
{
    if (type == "npcGreenArmoredTank") {
        GameObject *enemy = new GameObject("npcGreenArmoredTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setController(new StupidController(enemy));

        return enemy;
    }

    return nullptr;
}