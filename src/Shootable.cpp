#include "GameObject.h"
#include "Shootable.h"

Shootable::Shootable(GameObject *parent, int level, int timeout, int bulletSpeed)
    : _gameObject(parent)
    , _level(level)
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
    bullet->setRenderer(new SpriteRenderer(bullet), 2);
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


Shootable *Shootable::createDefaultPlayerShootable(GameObject *parent)
{
    return new Shootable(parent, 0, globalConst::PlayerShootTimeoutMs, globalConst::DefaultPlayerBulletSpeed);
}

Shootable *Shootable::createDefaultEnemyShootable(GameObject *parent)
{
    return new Shootable(parent, 0, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed);
}

Shootable *Shootable::createDefaultRocketShootable(GameObject *parent)
{
    return new RocketShootable(parent);
}

Shootable *Shootable::createDoubleRocketShootable(GameObject *parent)
{
    return new DoubleRocketShootable(parent);
}

///// Four direction

FourDirectionShootable::FourDirectionShootable(GameObject *parent, int bulletSpeed)
: Shootable(parent, 0, globalConst::PlayerShootTimeoutMs*2, bulletSpeed)
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
        bullet->setRenderer(new SpriteRenderer(bullet), 2);
        bullet->copyParentPosition(_gameObject);

        // add to bullet pool
        ObjectsPool::addObject(bullet);
    }

    return true;
}


RocketShootable::RocketShootable(GameObject *parent)
: Shootable(parent, 0, globalConst::RocketShootTimeoutMs, globalConst::DefaultRocketSpeed)
{}

bool RocketShootable::shoot(globalTypes::Direction dir)
{
    assert( dir != globalTypes::Direction::Unknown);
    if (isShootingProhibited())
        return false;
    _clock.restart();
    GameObject *bullet = new GameObject(_gameObject, "rocket");
    bullet->setParentId(_gameObject->id());
    bullet->setFlags(GameObject::Bullet | GameObject::Explosive);
    bullet->setController(new RocketController(bullet, dir, _bulletSpeed, _damage));
    bullet->setRenderer(new SpriteRenderer(bullet), 2);
    bullet->copyParentPosition(_gameObject);

    // add to bullet pool
    ObjectsPool::addObject(bullet);

    return true;
}

bool RocketShootable::isShootingProhibited() {
    if  (_clock.getElapsedTime() < sf::milliseconds(_actionTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("rocket");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > _level;
}

//////////////

DoubleShootable::DoubleShootable(GameObject *parent)
: Shootable(parent, 0, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed)
{}

bool DoubleShootable::shoot(globalTypes::Direction dir)
{
    using Direction =  globalTypes::Direction;
    assert( dir != Direction::Unknown);
    if (isShootingProhibited())
        return false;

    _clock.restart();

    sf::Vector2i pos = _gameObject->position();

    int offsetX = (dir == Direction::Up || dir == Direction::Down) ? 3 : 0;
    int offsetY = (dir == Direction::Left || dir == Direction::Right) ? 3 : 0;

    {
        GameObject *bullet = new GameObject(_gameObject, "bullet");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet);
        bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet), 2);
        bullet->setPosition(pos.x - offsetX, pos.y - offsetY);
        ObjectsPool::addObject(bullet);
    }
    {
        GameObject *bullet = new GameObject(_gameObject, "bullet");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet);
        bullet->setController(new BulletController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet), 2);
        bullet->setPosition(pos.x + offsetX, pos.y + offsetY);
        ObjectsPool::addObject(bullet);
    }

    // add to bullet pool

    return true;
}

bool DoubleShootable::isShootingProhibited() {
    if  (_clock.getElapsedTime() < sf::milliseconds(_actionTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > 1;
}

////

DoubleRocketShootable::DoubleRocketShootable(GameObject *parent)
: Shootable(parent, 0, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed * 4 / 3)
{}

bool DoubleRocketShootable::shoot(globalTypes::Direction dir)
{
    using Direction =  globalTypes::Direction;
    assert( dir != Direction::Unknown);
    if (isShootingProhibited())
        return false;

    _clock.restart();

    sf::Vector2i pos = _gameObject->position();

    int offsetX = (dir == Direction::Up || dir == Direction::Down) ? 4 : 0;
    int offsetY = (dir == Direction::Left || dir == Direction::Right) ? 4 : 0;

    {
        GameObject *bullet = new GameObject(_gameObject, "rocket");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet | GameObject::Explosive);
        bullet->setController(new RocketController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet), 2);
        bullet->setPosition(pos.x - offsetX, pos.y - offsetY);
        ObjectsPool::addObject(bullet);
    }
    {
        GameObject *bullet = new GameObject(_gameObject, "rocket");
        bullet->setParentId(_gameObject->id());
        bullet->setFlags(GameObject::Bullet | GameObject::Explosive);
        bullet->setController(new RocketController(bullet, dir, _bulletSpeed, _damage));
        bullet->setRenderer(new SpriteRenderer(bullet), 2);
        bullet->setPosition(pos.x + offsetX, pos.y + offsetY);
        ObjectsPool::addObject(bullet);
    }

    // add to bullet pool

    return true;
}

bool DoubleRocketShootable::isShootingProhibited() {
    if  (_clock.getElapsedTime() < sf::milliseconds(_actionTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("rocket");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > 1;
}


////

KamikazeShootable::KamikazeShootable(GameObject *parent) : Shootable(parent, 0, 0, 0) {}

bool KamikazeShootable::shoot(globalTypes::Direction)
{
    // in this case "shoot" is just self explode trying to harm target
    _gameObject->appendFlags(GameObject::Explosive);
    _gameObject->markForDeletion();
    return true;
}