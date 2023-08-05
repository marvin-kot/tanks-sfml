#include "Controller.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Utils.h"
#include "SoundPlayer.h"


Controller::Controller(GameObject *obj)
: _gameObject(obj) 
{}

StupidController::StupidController(GameObject *obj)
: Controller(obj), distribution(0, 4)
{}

void StupidController::update()
{
    if (clock.getElapsedTime() > actionTimeout) {
        clock.restart();
        // change decision
        int dir = distribution(Utils::generator);
        switch (dir) {
            case 0: // stay / shoot
                currMoveX = currMoveY = 0;
                isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->shoot();
                break;
            case 1: // left
                currMoveX = -moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Left);
                _gameObject->setCurrentAnimation("left");
                isMoving = true;
                break;
            case 2: // up
                currMoveY = -moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Up);
                _gameObject->setCurrentAnimation("up");
                isMoving = true;
                break;
            case 3: // right
                currMoveX = moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Right);
                _gameObject->setCurrentAnimation("right");
                isMoving = true;
                break;
            case 4: // down
                currMoveY = moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Down);
                _gameObject->setCurrentAnimation("down");
                isMoving = true;
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
        isMoving = false;
        if (_gameObject->shoot())
            SoundPlayer::instance().playShootSound();;
    } else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        _gameObject->move(-moveSpeed, 0);
        _gameObject->setCurrentDirection(globalTypes::Left);
        _gameObject->setCurrentAnimation("left");
        isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        _gameObject->move(0, -moveSpeed);
        _gameObject->setCurrentDirection(globalTypes::Up);
        _gameObject->setCurrentAnimation("up");
        isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        _gameObject->move(moveSpeed, 0);
        _gameObject->setCurrentDirection(globalTypes::Right);
        _gameObject->setCurrentAnimation("right");
        isMoving = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        _gameObject->move(0, moveSpeed);
        _gameObject->setCurrentDirection(globalTypes::Down);
        _gameObject->setCurrentAnimation("down");
        isMoving = true;
    } else
        isMoving = false;

    if (isMoving) {
        lastActionTime = clock.getElapsedTime();
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