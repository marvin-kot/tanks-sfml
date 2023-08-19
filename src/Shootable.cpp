#include "GameObject.h"
#include "Shootable.h"

Shootable::Shootable(GameObject *parent, int timeout, int bulletSpeed)
    : _gameObject(parent)
    , _actionTimeoutMs(timeout)
    , _bulletSpeed(bulletSpeed)
    , _damage(globalConst::DefaultDamage)
    {}

bool Shootable::shoot(globalTypes::Direction dir)
{
    assert( dir != globalTypes::Direction::Unknown);
    if (isShootingProhibited())
        return false;
    _clock.restart();
    GameObject *bullet = new GameObject(_gameObject, "bullet");
    bullet->setParentId(_gameObject->id());
    bullet->setFlags(GameObject::Bullet);
    /*if (_damage == 4) {
        bullet->setFlags(GameObject::PowerBullet);
    }*/
    bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
    bullet->setRenderer(new SpriteRenderer(bullet));
    bullet->copyParentPosition(_gameObject);

    // add to bullet pool
    ObjectsPool::addObject(bullet);

    return true;
}

bool Shootable::isShootingProhibited() {
    return _clock.getElapsedTime() < sf::milliseconds(_actionTimeoutMs);
}

////// PLayerShootable


PlayerShootable::PlayerShootable(GameObject *parent, int level)
: Shootable(parent, globalConst::PlayerShootTimeoutMs, globalConst::DefaultPlayerBulletSpeed), _level(level)
{}


bool PlayerShootable::isShootingProhibited()
{
    if (Shootable::isShootingProhibited()) return true;
    auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countPlayerBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countPlayerBullets++;
    }

    return countPlayerBullets > _level;
}


EnemyTankShootable::EnemyTankShootable(GameObject *parent)
: Shootable(parent, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed)
{}


bool EnemyTankShootable::isShootingProhibited()
{
    if (Shootable::isShootingProhibited()) return true;
    auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countPlayerBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countPlayerBullets++;
    }

    return countPlayerBullets > _level;
}