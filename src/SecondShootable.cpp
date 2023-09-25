#include "Damageable.h"
#include "GameObject.h"
#include "SecondShootable.h"
#include "SoundPlayer.h"
#include "Controller.h"

SecondShootable::SecondShootable(GameObject *parent)
: Shootable(parent, 0, 500, 0, 1)
{}

LandmineSetter::LandmineSetter(GameObject *parent)
: SecondShootable(parent)
{

}

bool LandmineSetter::isShootingProhibited()
{
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs))
        return true;

    auto mines = ObjectsPool::getObjectsByType("landmine");
    if (mines.size() >= _limit) {
        SoundPlayer::instance().enqueueSound(SoundPlayer::RejectLandmine, true);
        return true;
    }

    return false;
}

bool LandmineSetter::shoot(globalTypes::Direction)
{
    if (isShootingProhibited()) {
        return false;
    }

    _shootClock.reset(true);

    {
        GameObject *mine = new GameObject(_gameObject, "landmine");
        mine->setParentId(_gameObject->id());
        mine->setFlags(GameObject::TankPassable | GameObject::Explosive | GameObject::BulletPassable | GameObject::OwnedByPlayer);
        mine->setController(new LandmineController(mine, true));
        mine->setShootable(new KamikazeShootable(mine));
        mine->setRenderer(new SpriteRenderer(mine), 1);
        sf::Vector2i pos = _gameObject->position();
        mine->setPosition(pos.x, pos.y);
        ObjectsPool::addObject(mine);
        SoundPlayer::instance().enqueueSound(SoundPlayer::SetLandmine, true);
    }

    return true;
}


/////

TurretSetter::TurretSetter(GameObject *parent)
: SecondShootable(parent)
{

}

bool TurretSetter::isShootingProhibited()
{
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs))
        return true;

    auto activeTurrets = ObjectsPool::getObjectsByType("staticTurret");
    return (activeTurrets.size() >= _limit);
}

bool TurretSetter::shoot(globalTypes::Direction)
{
    if (isShootingProhibited())
        return false;

    _shootClock.reset(true);

    {
        GameObject *turret = new GameObject(_gameObject, "staticTurret");
        turret->setParentId(_gameObject->id());
        // WARNING: TankPassable flag is important to avoid collision with player tank after setting
        // once player leaves collision zone, the flag is unset (in StaticTurretController::update())
        turret->setFlags(GameObject::TankPassable | GameObject::BulletKillable | GameObject::OwnedByPlayer);
        turret->setController(new StaticTurretController(turret, _gameObject->direction()));
        turret->setShootable(new Shootable(turret, 0, globalConst::EnemyDefaultReloadTimeoutMs, globalConst::DefaultEnemyBulletSpeed, 4));
        turret->setRenderer(new SpriteRenderer(turret), 1);
        turret->setDamageable(new Damageable(turret, 0));
        sf::Vector2i pos = _gameObject->position();
        turret->setPosition(pos.x, pos.y);
        turret->setCurrentDirection(_gameObject->direction());
        ObjectsPool::addObject(turret);
    }

    return true;
}


//////////


BlockageSetter::BlockageSetter(GameObject *parent)
: SecondShootable(parent), _prot(2), _limit(4)
{

}

bool BlockageSetter::isShootingProhibited()
{
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs))
        return true;

    auto activeTurrets = ObjectsPool::getObjectsByType("blockage");
    return (activeTurrets.size() >= _limit);
}

bool BlockageSetter::shoot(globalTypes::Direction)
{
    if (isShootingProhibited())
        return false;

    _shootClock.reset(true);

    {
        GameObject *blockage = new GameObject(_gameObject, "blockage");
        blockage->setParentId(_gameObject->id());
        // WARNING: TankPassable flag is important to avoid collision with player tank after setting
        // once player leaves collision zone, the flag is unset (in StaticTurretController::update())
        blockage->setFlags(GameObject::TankPassable | GameObject::BulletKillable | GameObject::OwnedByPlayer);
        blockage->setController(new BlockageController(blockage));
        blockage->setRenderer(new SpriteRenderer(blockage), 1);
        blockage->setDamageable(new Damageable(blockage, _prot));
        sf::Vector2i pos = _gameObject->position();
        blockage->setPosition(pos.x, pos.y);
        blockage->setCurrentDirection(_gameObject->direction());
        ObjectsPool::addObject(blockage);
    }

    return true;
}


//////////


JezekSetter::JezekSetter(GameObject *parent)
: SecondShootable(parent), _paralyzeTime(10000), _limit(4)
{

}

bool JezekSetter::isShootingProhibited()
{
    if  (_shootClock.getElapsedTime() < sf::milliseconds(_shootTimeoutMs))
        return true;

    auto activeTurrets = ObjectsPool::getObjectsByType("playerJezek");
    return (activeTurrets.size() >= _limit);
}

bool JezekSetter::shoot(globalTypes::Direction)
{
    if (isShootingProhibited())
        return false;

    _shootClock.reset(true);

    {
        GameObject *jezek = new GameObject(_gameObject, "playerJezek");
        jezek->setParentId(_gameObject->id());
        // WARNING: TankPassable flag is important to avoid collision with player tank after setting
        // once player leaves collision zone, the flag is unset (in StaticTurretController::update())
        jezek->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static | GameObject::OwnedByPlayer);
        jezek->setController(new JezekController(jezek, true));
        jezek->setRenderer(new SpriteRenderer(jezek), 1);
        sf::Vector2i pos = _gameObject->position();
        jezek->setPosition(pos.x, pos.y);
        jezek->setCurrentDirection(_gameObject->direction());
        ObjectsPool::addObject(jezek);
    }

    return true;
}