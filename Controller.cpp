#include "Controller.h"
#include "GameObject.h"
#include "Utils.h"


Controller::Controller(AnimatedGameObject *obj)
: _gameObject(obj) 
{}

StupidController::StupidController(AnimatedGameObject *obj)
: Controller(obj), distribution(0, 4)
{}

void StupidController::update()
{
    if (clock.getElapsedTime() > actionTimeout) {
        clock.restart();
        // change decision
        int dir = distribution(Utils::generator);
        switch (dir) {
            case 0: // stay
                currMoveX = currMoveY = 0;
                isMoving = false;
                _gameObject->stopAnimation();
                break;
            case 1: // left
                currMoveX = -moveSpeed; currMoveY = 0;
                _gameObject->setCurrentAnimation("left");
                isMoving = true;
                break;
            case 2: // up
                currMoveY = -moveSpeed; currMoveX = 0;
                _gameObject->setCurrentAnimation("up");
                isMoving = true;
                break;
            case 3: // right
                currMoveX = moveSpeed; currMoveY = 0;
                _gameObject->setCurrentAnimation("right");
                isMoving = true;
                break;
            case 4: // down
                currMoveY = moveSpeed; currMoveX = 0;
                _gameObject->setCurrentAnimation("down");
                isMoving = true;
                break;
        }
    }

    _gameObject->move(currMoveX, currMoveY);
}

PlayerController::PlayerController(AnimatedGameObject *obj)
: Controller(obj)
{}

void PlayerController::update()
{
    bool action = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        _gameObject->move(-moveSpeed, 0);
        _gameObject->setCurrentAnimation("left");
        action = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        _gameObject->move(0, -moveSpeed);
        _gameObject->setCurrentAnimation("up");
        action = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        _gameObject->move(moveSpeed, 0);
        _gameObject->setCurrentAnimation("right");
        action = true;
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        _gameObject->move(0, moveSpeed);
        _gameObject->setCurrentAnimation("down");
        action = true;
    }

    if (action) {
        lastActionTime = clock.getElapsedTime();
        isMoving = true;
    } else {
        isMoving = false;
        _gameObject->stopAnimation();
    }
}