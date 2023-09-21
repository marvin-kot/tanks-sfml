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
        mine->setRenderer(new SpriteRenderer(mine), 2);
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
    if (activeTurrets.size() >= _limit) {
        SoundPlayer::instance().enqueueSound(SoundPlayer::RejectLandmine, true);
        return true;
    }

    return false;
}

bool TurretSetter::shoot(globalTypes::Direction)
{
    if (isShootingProhibited()) {
        return false;
    }

    _shootClock.reset(true);

    {
        GameObject *turret = new GameObject(_gameObject, "staticTurret");
        turret->setParentId(_gameObject->id());
        // WARNING: TankPassable flag is important to avoid collision with player tank after setting
        // once player leaves collision zone, the flag is unset (in StaticTurretController::update())
        turret->setFlags(GameObject::TankPassable | GameObject::BulletKillable | GameObject::OwnedByPlayer);
        turret->setController(new StaticTurretController(turret, _gameObject->direction()));
        turret->setShootable(new Shootable(turret, 0, globalConst::EnemyDefaultReloadTimeoutMs, globalConst::DefaultEnemyBulletSpeed, 4));
        turret->setRenderer(new SpriteRenderer(turret), 2);
        turret->setDamageable(new Damageable(turret, 0));
        sf::Vector2i pos = _gameObject->position();
        turret->setPosition(pos.x, pos.y);
        turret->setCurrentDirection(_gameObject->direction());
        ObjectsPool::addObject(turret);
        SoundPlayer::instance().enqueueSound(SoundPlayer::SetLandmine, true);
    }

    return true;
}