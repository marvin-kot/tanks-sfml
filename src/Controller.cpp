#include "Controller.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "Utils.h"

#include <algorithm>


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
    assert(dir != globalTypes::Direction::Unknown);
    _gameObject->setCurrentDirection(dir);

    switch (dir) {
        case globalTypes::Left:
            _currMoveX = -speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("left");
            if (_gameObject->turret)
                _gameObject->turret->setCurrentAnimation("left");
            break;
        case globalTypes::Up:
            _currMoveY = -speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("up");
            if (_gameObject->turret)
                _gameObject->turret->setCurrentAnimation("up");
            break;
        case globalTypes::Right: // right
            _currMoveX = speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("right");
            if (_gameObject->turret)
                _gameObject->turret->setCurrentAnimation("right");
            break;
        case globalTypes::Down: // down
            _currMoveY = speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("down");
            if (_gameObject->turret)
                _gameObject->turret->setCurrentAnimation("down");
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





/////

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg, bool pierce)
: Controller(obj, spd), _direction(dir), _damage(dmg), _piercing(pierce)
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
    assert(_direction != globalTypes::Direction::Unknown );
    prepareMoveInDirection(_direction, speed);
    _gameObject->move(_currMoveX, _currMoveY);

}


/////

RocketController::RocketController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: BulletController(obj, dir, spd, dmg)
{
    _currSpeed = _startSpeed = globalConst::DefaultPlayerBulletSpeed/4;
    _clock.reset(true);
    _maxSpeed = _moveSpeed;
}

void RocketController::update()
{

    checkForGamePause();
    if (_pause) return;


    if (_clock.getElapsedTime() > sf::milliseconds(globalConst::DefaultBulletLifetimeMs))
        _gameObject->markForDeletion();

    _moveSpeed = _currSpeed;
    int speed = moveSpeedForCurrentFrame();
    assert(_direction != globalTypes::Direction::Unknown );
    prepareMoveInDirection(_direction, speed);
    _gameObject->move(_currMoveX, _currMoveY);

    _currSpeed += (_maxSpeed - _startSpeed)/20;
}




ExplosionController::ExplosionController(GameObject *parent, bool dontHurtParent)
: Controller(parent, 0), _dontHurtParent(dontHurtParent)
{
    _clock.reset(true);
}


void ExplosionController::update()
{
    checkForGamePause();
    if (_pause) return;

    _gameObject->move(0, 0);
}