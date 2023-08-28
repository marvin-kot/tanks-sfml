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
    if (_piercing) {
        bullet->appendFlags(GameObject::PiercingBullet);
    }
    bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage, _piercing));
    bullet->setRenderer(new SpriteRenderer(bullet));
    bullet->copyParentPosition(_gameObject);

    // add to bullet pool
    ObjectsPool::addObject(bullet);

    return true;
}

bool Shootable::isShootingProhibited() {
    if  (_clock.getElapsedTime() < sf::milliseconds(_actionTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > _level;
}

////// PLayerShootable


PlayerShootable::PlayerShootable(GameObject *parent, int level)
: Shootable(parent, globalConst::PlayerShootTimeoutMs, globalConst::DefaultPlayerBulletSpeed)
{
    _level = level;
}

//////

EnemyTankShootable::EnemyTankShootable(GameObject *parent)
: Shootable(parent, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed)
{}

///// Four direction

FourDirectionShootable::FourDirectionShootable(GameObject *parent)
: PlayerShootable(parent, 0)
{
}



bool FourDirectionShootable::shoot(globalTypes::Direction)
{
    // direction param here does not matter and exist only for compatibility

    using namespace globalTypes;
    bool madeShot = false;
    if (isShootingProhibited()) return false;
    _clock.restart();

    for (Direction dir = Up; dir <= Right; dir = (Direction)((int)dir + 1)) {
        GameObject *bullet = new GameObject(_gameObject, "bullet");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet);
        if (_piercing)
            bullet->appendFlags(GameObject::PiercingBullet);

        bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage, _piercing));
        bullet->setRenderer(new SpriteRenderer(bullet));
        bullet->copyParentPosition(_gameObject);

        // add to bullet pool
        ObjectsPool::addObject(bullet);
    }

    return true;
}