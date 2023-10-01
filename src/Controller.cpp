#include "Controller.h"
#include "Collectable.h"
#include "Damageable.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "SoundPlayer.h"
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

void Controller::paralyze(int msec)
{
    assert(_gameObject != nullptr);
    if (_gameObject->isFlagSet(GameObject::Player))
        msec /= 2;

    _paralyzedForMs = msec;
    _blinkClock.reset(true);
    _paralyzeClock.reset(true);
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
        _paralyzeClock.pause();
        _blinkClock.pause();
        _pause = true;
    } else if (_pause && ! globalVars::gameIsPaused) {
        _clock.resume();
        _paralyzeClock.resume();
        _blinkClock.resume();
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

void BulletController::onCollided(GameObject *other)
{
    if (_updatedOnFrame == Utils::currentFrame) return;

    if (other->isFlagSet(GameObject::BulletPassable))
        return;

    // bullet cannot hit its parent
    if (_gameObject->parentId() == other->id())
        return;

    _updatedOnFrame = Utils::currentFrame;
}


GameObject *BulletController::onDestroyed()
{
    if (_gameObject->getParentObject() && _gameObject->getParentObject()->isFlagSet(GameObject::Player))
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);

    if (_gameObject->isFlagSet(GameObject::Explosive))
        return ExplosionController::createBigExplosion(_gameObject, true);
    else
        return ExplosionController::createSmallExplosion(_gameObject);
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


GameObject *ExplosionController::createSmallExplosion(GameObject *parent)
{
    GameObject *explosion = new GameObject("smallExplosion");
    explosion->setRenderer(new OneShotAnimationRenderer(explosion), 4);
    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
    explosion->copyParentPosition(parent);

    return explosion;
}

GameObject *ExplosionController::createBigExplosion(GameObject *parent, bool damaging)
{
    GameObject *explosion = new GameObject("bigExplosion");
    if (damaging) {
        explosion->setController(new ExplosionController(explosion, false));
        explosion->damage = 1;
    }
    explosion->setRenderer(new OneShotAnimationRenderer(explosion), 4);
    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
    explosion->copyParentPosition(parent);
    explosion->setParentId(parent->parentId());
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);

    return explosion;
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

    if (obj->isFlagSet(GameObject::ObjectFlags::BulletKillable)) {
        _gameObject->shoot();
    }
}


GameObject *LandmineController::onDestroyed()
{
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);
    // true - damaging explosion
    return ExplosionController::createBigExplosion(_gameObject, true);
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

//////


BlockageController::BlockageController(GameObject *parent)
: Controller(parent, 0) {}


void BlockageController::update()
{
    checkForGamePause();

    if (_pause)
        return;

    if (nullptr == ObjectsPool::playerObject || !_gameObject->collides(*ObjectsPool::playerObject))
        _gameObject->unsetFlags(GameObject::TankPassable);
}



void BlockageController::onDamaged() {
    assert(_gameObject != nullptr);
    auto damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);
    assert(_gameObject->spriteRenderer != nullptr);
    switch (damageable->defence())
    {
        case 1:
            _gameObject->spriteRenderer->setCurrentAnimation("damaged");
            break;
        case 0:
            _gameObject->spriteRenderer->setCurrentAnimation("damaged-2");
            break;
        default:
            _gameObject->spriteRenderer->setCurrentAnimation("default");
    }
}


JezekController::JezekController(GameObject *parent, bool dontHurtParent)
: Controller(parent, 0), _dontHurtParent(dontHurtParent) {}


void JezekController::update()
{
    checkForGamePause();
    if (_pause) return;

    _gameObject->move(0, 0);
}

void JezekController::onCollided(GameObject *obj)
{
    assert(_gameObject != nullptr);

    if (_dontHurtParent && (obj->id() == _gameObject->parentId() || obj->parentId() == _gameObject->parentId()))
        return;

    if (obj->isFlagSet(GameObject::ObjectFlags::BulletKillable)) {
        _gameObject->markForDeletion();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::DestroyWall, true);
        obj->getComponent<Controller>()->paralyze(10000);
    }

}



SkullController::SkullController(GameObject *parent, int timeout)
: Controller(parent, 0), _timeoutMsec(timeout) {
    _clock.reset(true);
}

void SkullController::update()
{
    checkForGamePause();
    if (_pause) return;

    if (_clock.getElapsedTime() > sf::milliseconds(_timeoutMsec)) {
        _gameObject->markForDeletion();
    }
}

////

CollectableXpController::CollectableXpController(GameObject *parent)
: Controller(parent, 0) {}

void CollectableXpController::onCollided(GameObject *other)
{
    assert(_gameObject->isFlagSet(GameObject::CollectableBonus));

    if (!other->isFlagSet(GameObject::CollectableBonus)) return;

    // merge colliding xp points to reduce number of objects on map
    auto thisCollectable = _gameObject->getComponent<Collectable>();
    assert(thisCollectable != nullptr);
    XpCollectable *thisXp = dynamic_cast<XpCollectable *>(thisCollectable);
    if (thisXp == nullptr)
        return;

    auto thatCollectable = other->getComponent<Collectable>();
    assert(thatCollectable != nullptr);

    XpCollectable *thatXp = dynamic_cast<XpCollectable *>(thatCollectable);
    if (thatXp == nullptr)
        return;

    int sum = thisXp->value() + thatXp->value();
    if (sum > 1000 || (sum % 100 != 0))
        return; // 900 is maximum xp collectable...

    // update value
    thisXp->setValue(sum);
    std::string newType;
    switch (sum) {
        case 200: newType = "200xp"; break;
        case 300: newType = "300xp"; break;
        case 400: newType = "400xp"; break;
        case 500: newType = "500xp"; break;
        case 600: newType = "600xp"; break;
        case 700: newType = "700xp"; break;
        case 800: newType = "800xp"; break;
        case 900: newType = "900xp"; break;
        case 1000: newType = "1000xp"; break;
        default:
            return;
    }

    assert(_gameObject->spriteRenderer != nullptr);
    _gameObject->spriteRenderer->setNewObjectType(newType);

    // delete second object
    other->markForDeletion();
}


////

StaticCarController::StaticCarController(GameObject *parent)
: Controller(parent, 0) {
    _clock.reset(true);
}

void StaticCarController::onCollided(GameObject *other)
{
    assert(other != nullptr);
    if (other->isFlagSet(GameObject::Player) || other->isFlagSet(GameObject::NPC)) {
        _gameObject->markForDeletion();
        SoundPlayer::instance().enqueueSound(SoundPlayer::DestroyWall, true);
    }
}

GameObject *StaticCarController::onDestroyed()
{
    //SoundPlayer::instance().enqueueSound(SoundPlayer::DestroyWall, true);
    return ExplosionController::createSmallExplosion(_gameObject);
}

////

////

PrizeBoxController::PrizeBoxController(GameObject *parent)
: Controller(parent, 0) {
}


GameObject *PrizeBoxController::onDestroyed()
{
    SoundPlayer::instance().enqueueSound(SoundPlayer::DestroyWall, true);
    _gameObject->generateDrop();
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusAppear, true);

    return ExplosionController::createSmallExplosion(_gameObject);
}