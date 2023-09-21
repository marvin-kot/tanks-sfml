#include "Controller.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "ObjectsPool.h"
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
            break;
        case globalTypes::Up:
            _currMoveY = -speed; _currMoveX = 0;
            break;
        case globalTypes::Right:
            _currMoveX = speed; _currMoveY = 0;
            break;
        case globalTypes::Down:
            _currMoveY = speed; _currMoveX = 0;
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


/////////


LandmineController::LandmineController(GameObject *parent, bool dontHurtParent)
: Controller(parent, 0), _dontHurtParent(dontHurtParent)
{
    _clock.reset(true);
}


void LandmineController::update()
{
    checkForGamePause();
    if (_pause) return;

    _gameObject->move(0, 0);
}

void LandmineController::onCollided(GameObject *obj)
{
    if (_dontHurtParent && (obj->id() == _gameObject->parentId() || obj->parentId() == _gameObject->parentId()))
        return;

    if (obj->isFlagSet(GameObject::ObjectFlags::BulletKillable))
        _gameObject->shoot();
}

///

StaticTurretController::StaticTurretController(GameObject *parent, globalTypes::Direction dir)
: Controller(parent, 0), _direction(dir)
{
    _clock.reset(true);
}

void StaticTurretController::update()
{
    checkForGamePause();

    if (_pause)
        return;

    _gameObject->move(0, 0);

    if (nullptr == ObjectsPool::playerObject || !_gameObject->collides(*ObjectsPool::playerObject))
        _gameObject->unsetFlags(GameObject::TankPassable);

    if (decideIfToShoot())
        _gameObject->shoot();
}

void StaticTurretController::onCollided(GameObject *obj)
{
}

bool StaticTurretController::decideIfToShoot() const
{
    GameObject *hit = _gameObject->linecastInCurrentDirection();

    if (hit == nullptr)
        return false;

    bool targetIsPlayer = false;
    if (ObjectsPool::playerObject && _gameObject->parentId() != ObjectsPool::playerObject->id())
        targetIsPlayer = true;

    if (targetIsPlayer && (hit == ObjectsPool::playerObject || hit == ObjectsPool::eagleObject)) {
        return true;
    }

    if (!targetIsPlayer && hit->isFlagSet(GameObject::NPC)) {
        return true;
    }


    return false;
}