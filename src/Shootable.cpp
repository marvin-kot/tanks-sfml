#include "GameObject.h"
#include "Shootable.h"

Shootable::Shootable(GameObject *parent, int timeout)
    : _gameObject(parent)
    , _actionTimeoutMs(timeout)
    , _bulletSpeed(globalConst::DefaultBulletSpeed)
    , _damage(globalConst::DefaultDamage)
    {}

bool Shootable::shoot(globalTypes::Direction dir)
{
    if (isShootingProhibited())
        return false;
    _clock.restart();
    Logger::instance() << "shoot";
    GameObject *bullet = new GameObject(_gameObject, "bullet");
    bullet->setParentId(_gameObject->id());
    bullet->setFlags(GameObject::Bullet);
    if (_damage == 2) {
        bullet->setFlags(GameObject::PiercingBullet);
    }
    bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
    bullet->setRenderer(new SpriteRenderer(bullet));
    bullet->copyParentPosition(_gameObject);

    // add to bullet pool
    ObjectsPool::addObject(bullet);

    return true;
}

bool Shootable::isShootingProhibited() {
    return sf::milliseconds(_actionTimeoutMs) > _clock.getElapsedTime();
}

////// PLayerShootable


PlayerShootable::PlayerShootable(GameObject *parent, int level)
: Shootable(parent, globalConst::PlayerShootTimeoutMs), _level(level)
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
