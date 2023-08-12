#include "Controller.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "Damageable.h"
#include "Logger.h"


Controller::Controller(GameObject *parent)
: _gameObject(parent)
{}

/////

TankRandomController::TankRandomController(GameObject *parent, int speed, float timeoutSec)
: Controller(parent), _moveSpeed(speed), _actionTimeout(sf::seconds(timeoutSec)),distribution(1, 4)
{}

void TankRandomController::update()
{
    {
        using namespace globalVars;
        if (globalTimeFreeze) {
            if (globalFreezeClock.getElapsedTime() < sf::seconds(10)) {
                _currMoveX = _currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->move(_currMoveX, _currMoveY);
                return;
            }
            else
                globalTimeFreeze = false;
        }
    }


    int tries = 4;
    int moved = 0; // TODO remove magic numbers
    bool resetTimeout = false;
    do {
        if (_clock.getElapsedTime() > _actionTimeout) {
            // change decision
            resetTimeout = true;
            int dir = distribution(Utils::generator);
            int speed = ((int)(_moveSpeed * Utils::lastFrameTime.asSeconds()) << 1) >> 1;
            switch (dir) {
                case 1: // left
                    _currMoveX = -speed; _currMoveY = 0;
                    _gameObject->setCurrentDirection(globalTypes::Left);
                    _gameObject->setCurrentAnimation("left");
                    _isMoving = true;
                    break;
                case 2: // up
                    _currMoveY = -speed; _currMoveX = 0;
                    _gameObject->setCurrentDirection(globalTypes::Up);
                    _gameObject->setCurrentAnimation("up");
                    _isMoving = true;
                    break;
                case 3: // right
                    _currMoveX = speed; _currMoveY = 0;
                    _gameObject->setCurrentDirection(globalTypes::Right);
                    _gameObject->setCurrentAnimation("right");
                    _isMoving = true;
                    break;
                case 4: // down
                    _currMoveY = speed; _currMoveX = 0;
                    _gameObject->setCurrentDirection(globalTypes::Down);
                    _gameObject->setCurrentAnimation("down");
                    _isMoving = true;
                    break;
            }
        }

        moved = _gameObject->move(_currMoveX, _currMoveY);
    } while (resetTimeout && --tries && moved == 0);

    if (resetTimeout) {
        _clock.restart();
        int shotChance = distribution(Utils::generator);
        if (shotChance > 2)
            _gameObject->shoot();
    }



}

/////

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: Controller(obj), _direction(dir), _moveSpeed(spd), _damage(dmg)
{}

void BulletController::update()
{
    int speed = (int)((float)_moveSpeed * Utils::lastFrameTime.asSeconds());
    if (_direction == globalTypes::Left)
    {
        _gameObject->move(-speed, 0);
        _gameObject->setCurrentAnimation("left");
    } else if (_direction == globalTypes::Up) {
        _gameObject->move(0, -speed);
        _gameObject->setCurrentAnimation("up");
    } else if (_direction == globalTypes::Right) {
        _gameObject->move(speed, 0);
        _gameObject->setCurrentAnimation("right");
    } else if (_direction == globalTypes::Down) {
        _gameObject->move(0, speed);
        _gameObject->setCurrentAnimation("down");
    }
}





