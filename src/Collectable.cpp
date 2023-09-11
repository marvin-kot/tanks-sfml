#include "Collectable.h"

#include "Damageable.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "SpriteRenderer.h"
#include "PlayerController.h"
#include "GlobalConst.h"

Collectable::Collectable(GameObject *parent)
: _gameObject(parent)
{}

void Collectable::onCollected(GameObject *collector)
{
    (void *)collector;
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusCollect, true);
}

///////

GrenadeCollectable::GrenadeCollectable(GameObject *parent)
: Collectable(parent)
{}

void GrenadeCollectable::onCollected(GameObject *collector)
{
    // kill all enemy tanks in a moment
    std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcKamikazeTank", "npc4DirTank", "npc4DirArmorTank", "npcDoubleCannonArmorTank"});
    std::for_each(objectsToKill.cbegin(), objectsToKill.cend(), [](GameObject *obj) { obj->markForDeletion(); });

    Collectable::onCollected(collector);
}

//////

TimerCollectable::TimerCollectable(GameObject *parent)
: Collectable(parent)
{}

void TimerCollectable::onCollected(GameObject *collector)
{
    globalVars::globalTimeFreeze = true;
    globalVars::globalFreezeChronometer.reset(true);

    Collectable::onCollected(collector);
}


//////

HelmetCollectable::HelmetCollectable(GameObject *parent)
: Collectable(parent)
{}

void HelmetCollectable::onCollected(GameObject *collector)
{
    PlayerController *controller = collector->getComponent<PlayerController>();
    controller->setTemporaryInvincibility(5000);

    Collectable::onCollected(collector);
}

/////

TankCollectable::TankCollectable(GameObject *parent)
: Collectable(parent)
{}

void TankCollectable::onCollected(GameObject *collector)
{
    assert(collector != nullptr);
    assert(collector->isFlagSet(GameObject::Player));

    auto spawnerObject = collector->getParentObject();
    assert(spawnerObject != nullptr);

    auto spawnerController = spawnerObject->getComponent<PlayerSpawnController>();

    assert(spawnerController != nullptr);

    spawnerController->appendLife();

    Collectable::onCollected(collector);
}


XpCollectable::XpCollectable(GameObject *parent, int value)
: Collectable(parent), _value(value)
{}

void XpCollectable::onCollected(GameObject *collector)
{
    PlayerController *controller = collector->getComponent<PlayerController>();
    controller->addXP(_value);

    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::xpCollect, true);
}

//////

AmmoCollectable::AmmoCollectable(GameObject *parent)
: Collectable(parent)
{}

void AmmoCollectable::onCollected(GameObject *collector)
{
    assert(collector->isFlagSet(GameObject::Player));
    //PlayerController *controller = collector->getComponent<PlayerController>();


    auto shootable = collector->getComponent<Shootable>();
    if (shootable->bullets() < shootable->maxBullets()) {
        shootable->resetBullets();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::FullReload, true);
    }

    //Collectable::onCollected(collector);
}


//////

RepairCollectable::RepairCollectable(GameObject *parent)
: Collectable(parent)
{}

void RepairCollectable::onCollected(GameObject *collector)
{
    assert(collector->isFlagSet(GameObject::Player));
    auto damageable = collector->getComponent<Damageable>();
    assert(damageable != nullptr);
    if (damageable->defence() < damageable->maxDefence()) {
        damageable->restoreDefence();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusCollect, true);
        auto controller = collector->getComponent<PlayerController>();
        controller->updateAppearance();
    }
}