#include "GameObject.h"
#include "Shootable.h"
#include "SoundPlayer.h"

Shootable::Shootable(GameObject *parent, int level, int timeout, int bulletSpeed, int maxBullets)
    : _gameObject(parent)
    , _level(level)
    , _shootTimeoutMs(timeout)
    , _bulletSpeed(bulletSpeed)
    , _damage(globalConst::DefaultDamage)
    , _maxBullets(maxBullets)
    {
        resetBullets();
        _shootClock.reset(true);
    }

bool Shootable::shoot(globalTypes::Direction dir)
{
    assert( dir != globalTypes::Direction::Unknown);
    if (isShootingProhibited())
        return false;
    _shootClock.reset(true);
    _reloadClock.reset(true);
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

    if (_bullets == _maxBullets) _reloadClock.reset(true);

    _bullets--;

    return true;
}

void Shootable::checkForGamePause()
{
    if (!_pause && globalVars::gameIsPaused) {
        _shootClock.pause();
        _reloadClock.pause();

        _pause = true;
    } else if (_pause && ! globalVars::gameIsPaused) {
        _shootClock.resume();
        _reloadClock.resume();
        _pause = false;
    }
}


void Shootable::reloadByTimeout()
{
    checkForGamePause();
    if (_pause) return;

    if (_bullets == _maxBullets) return;
    if  (_reloadClock.getElapsedTime() < sf::milliseconds(_reloadTimeoutMs)) return;

    bool isPlayer = _gameObject->isFlagSet(GameObject::Player);


    //if (_instantReload) {
    if (_bullets == 0) {
        _bullets = _maxBullets;
        if (isPlayer)
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::FullReload, true);
    }
    else {
        _bullets++;
        if (isPlayer)
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::PartialReload, true);
    }
    _reloadClock.reset(true);
}

bool Shootable::isShootingProhibited() {
    if (_bullets < 1) return true;
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs)) return true;

    /*auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > _level;*/

    return false;
}


Shootable *Shootable::createDefaultPlayerShootable(GameObject *parent)
{
    using namespace globalConst;
    auto shootable = new Shootable(parent, 0, PlayerShootTimeoutMs, DefaultPlayerBulletSpeed, PlayerDefaultMaxBullets);
    shootable->setReloadTimeoutMs(PlayerDefaultReloadTimeoutMs);

    return shootable;
}

Shootable *Shootable::createDefaultEnemyShootable(GameObject *parent)
{
    using namespace globalConst;
    auto shootable = new Shootable(parent, 0, EnemyShootTimeoutMs, DefaultEnemyBulletSpeed, EnemyDefaultMaxBullets);
    shootable->setReloadTimeoutMs(EnemyDefaultReloadTimeoutMs);

    return shootable;
}

Shootable *Shootable::createDefaultRocketShootable(GameObject *parent)
{
    auto shootable = new RocketShootable(parent, globalConst::PlayerDefaultMaxBullets);
    shootable->setReloadTimeoutMs(globalConst::PlayerDefaultReloadTimeoutMs);
    return shootable;
}

Shootable *Shootable::createDoubleRocketShootable(GameObject *parent)
{
    auto shootable = new DoubleRocketShootable(parent, globalConst::PlayerDefaultMaxBullets);
    shootable->setReloadTimeoutMs(globalConst::PlayerDefaultReloadTimeoutMs);

    return shootable;
}

///// Four direction

FourDirectionShootable::FourDirectionShootable(GameObject *parent, int bulletSpeed, int maxBullets)
: Shootable(parent, 0, globalConst::PlayerShootTimeoutMs*2, bulletSpeed, maxBullets)
{
}


bool FourDirectionShootable::shoot(globalTypes::Direction)
{
    // direction param here does not matter and exist only for compatibility

    using namespace globalTypes;
    bool madeShot = false;
    if (isShootingProhibited()) return false;
    _shootClock.reset(true);

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


RocketShootable::RocketShootable(GameObject *parent, int maxBullets)
: Shootable(parent, 0, globalConst::RocketShootTimeoutMs, globalConst::DefaultRocketSpeed, maxBullets)
{}

bool RocketShootable::shoot(globalTypes::Direction dir)
{
    assert( dir != globalTypes::Direction::Unknown);
    if (isShootingProhibited())
        return false;
    _shootClock.reset(true);
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
    return Shootable::isShootingProhibited();
    /*
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("rocket");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > _level;
    */
}

//////////////

DoubleShootable::DoubleShootable(GameObject *parent, int maxBullets)
: Shootable(parent, 0, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed, maxBullets)
{}

bool DoubleShootable::shoot(globalTypes::Direction dir)
{
    using Direction =  globalTypes::Direction;
    assert( dir != Direction::Unknown);
    if (isShootingProhibited())
        return false;

    _shootClock.reset(true);

    sf::Vector2i pos = _gameObject->position();

    int offsetX = (dir == Direction::Up || dir == Direction::Down) ? 5 : 0;
    int offsetY = (dir == Direction::Left || dir == Direction::Right) ? 5 : 0;

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
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("bullet");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > 1;
}

////

DoubleRocketShootable::DoubleRocketShootable(GameObject *parent, int maxBullets)
: Shootable(parent, 0, globalConst::EnemyShootTimeoutMs, globalConst::DefaultEnemyBulletSpeed * 4 / 3, maxBullets)
{}

bool DoubleRocketShootable::shoot(globalTypes::Direction dir)
{
    using Direction =  globalTypes::Direction;
    assert( dir != Direction::Unknown);
    if (isShootingProhibited())
        return false;

    _shootClock.reset(true);

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
    return Shootable::isShootingProhibited();
    /*
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs)) return true;
    auto bullets = ObjectsPool::getObjectsByType("rocket");
    int countMyBullets = 0;

    for (auto b : bullets) {
        if (b && !b->mustBeDeleted() && b->parentId() == _gameObject->id())
            countMyBullets++;
    }

    return countMyBullets > 1;*/
}


////

KamikazeShootable::KamikazeShootable(GameObject *parent) : Shootable(parent, 0, 0, 0, 0) {}

bool KamikazeShootable::shoot(globalTypes::Direction)
{
    // in this case "shoot" is just self explode trying to harm target
    _gameObject->appendFlags(GameObject::Explosive);
    _gameObject->markForDeletion();
    return true;
}