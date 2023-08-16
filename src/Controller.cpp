#include "Controller.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "Damageable.h"
#include "Logger.h"


Controller::Controller(GameObject *parent, int spd)
: _gameObject(parent), _moveSpeed(spd)
{}

void Controller::prepareMoveInDirection(globalTypes::Direction dir)
{
    _gameObject->setCurrentDirection(dir);
    int speed = ((int)(_moveSpeed * Utils::lastFrameTime.asSeconds()) << 1) >> 1;
    switch (dir) {
        case globalTypes::Left:
            _currMoveX = -speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("left");
            break;
        case globalTypes::Up:
            _currMoveY = -speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("up");
            break;
        case globalTypes::Right: // right
            _currMoveX = speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("right");
            break;
        case globalTypes::Down: // down
            _currMoveY = speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("down");
            break;
        default:
            _currMoveY = 0; _currMoveX = 0;
    }
}

void Controller::checkForGamePause()
{
    if (!_pause && globalVars::gameIsPaused) {
        _clock.pause();
        _pause = true;
    } else if (_pause && ! globalVars::gameIsPaused) {
        _clock.resume();
        _pause = false;
    }
}

/////

TankRandomController::TankRandomController(GameObject *parent, int spd, float timeoutSec)
: Controller(parent, spd), _actionTimeout(sf::seconds(timeoutSec)),distribution(1, 4)
{
    _clock.reset(true);
}

void TankRandomController::update()
{
    checkForGamePause();

    if (_pause)
        return;

    {
        using namespace globalVars;
        if (globalTimeFreeze) {
            if (globalFreezeChronometer.getElapsedTime() < sf::seconds(globalFreezeTimeout)) {
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
            globalTypes::Direction dir = static_cast<globalTypes::Direction> (distribution(Utils::generator));
            prepareMoveInDirection(dir);
        }

        moved = _gameObject->move(_currMoveX, _currMoveY);
    } while (resetTimeout && --tries && moved == 0);

    _isMoving = (moved == 1);

    if (resetTimeout) {
        _clock.reset(true);
        int shotChance = distribution(Utils::generator);
        if (shotChance > 2)
            _gameObject->shoot();
    }



}

/////

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: Controller(obj, spd), _direction(dir), _damage(dmg)
{
    _clock.reset(true);
}

void BulletController::update()
{
    checkForGamePause();
    if (_pause) return;

    if (_clock.getElapsedTime() > sf::milliseconds(globalConst::DefaultBulletLifetimeMs))
        _gameObject->markForDeletion();

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


/////

