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

int Controller::moveSpeedForCurrentFrame()
{
    const float fSpeed = (float)_moveSpeed * Utils::lastFrameTime.asSeconds();
    int speed = std::floor(fSpeed);
    float fraction = fSpeed - speed;

    if (fraction >= 0.1 && fraction < 0.3) {
        if (Utils::currentFrame % 4 == 0)
            speed++;
    } else if (fraction >=0.3 && fraction < 0.4) {
        if (Utils::currentFrame % 3 == 0)
            speed++;
    } else if (fraction >=0.4 && fraction < 0.6) {
        if (Utils::currentFrame % 2 == 0)
            speed++;
    } else if (fraction >=0.6 && fraction < 0.8) {
        if (Utils::currentFrame % 3 != 0)
            speed++;
    } else if (fraction >= 0.8) {
        if (Utils::currentFrame % 4 != 0)
            speed++;
    }

    return speed;
}

void Controller::prepareMoveInDirection(globalTypes::Direction dir, int speed)
{
    _gameObject->setCurrentDirection(dir);

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


    int tries = 1;
    int moved = 0; // TODO remove magic numbers
    bool resetTimeout = false;
    int speed = moveSpeedForCurrentFrame();
    do {
        globalTypes::Direction dir = globalTypes::Unknown;
        if (_clock.getElapsedTime() <= _actionTimeout)
            dir = _gameObject->direction();
        else {
            // change decision
            resetTimeout = true;
            dir = static_cast<globalTypes::Direction> (distribution(Utils::generator));
        }

        prepareMoveInDirection(dir, speed);


        moved = _gameObject->move(_currMoveX, _currMoveY);
        if (moved == 0) {
            // try same direction but +1/-1 pixes aside
            moved = trySqueeze();
        }
    } while (resetTimeout && --tries && moved == 0);

    _isMoving = (moved == 1);

    if (resetTimeout) {
        _clock.reset(true);
        int shotChance = distribution(Utils::generator);
        if (shotChance > 2)
            _gameObject->shoot();
    }
}

int TankRandomController::trySqueeze()
{
    int moved = 1;
    if (_currMoveX == 0) {
        moved = _gameObject->move(2, _currMoveY);
        if (moved == 0)
            moved = _gameObject->move(-2, _currMoveY);
    } else if (_currMoveY == 0) {
        moved = _gameObject->move(_currMoveX, 2);
        if (moved == 0)
            moved = _gameObject->move(_currMoveX, -2);
    }

    return 0;
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

    int speed = moveSpeedForCurrentFrame();
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

