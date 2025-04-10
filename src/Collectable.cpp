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

bool Collectable::onCollected(GameObject *collector)
{
    (void *)collector;
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusCollect, true);
    return true;
}

GrenadeCollectable::GrenadeCollectable(GameObject *parent)
: Collectable(parent)
{}

bool GrenadeCollectable::onCollected(GameObject *collector)
{
    std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcKamikazeTank", "npc4DirTank", "npc4DirArmorTank", "npcDoubleCannonArmorTank"});
    std::for_each(objectsToKill.cbegin(), objectsToKill.cend(), [](GameObject *obj) { obj->markForDeletion(); });

    Collectable::onCollected(collector);
    return true;
}

TimerCollectable::TimerCollectable(GameObject *parent)
: Collectable(parent)
{}

bool TimerCollectable::onCollected(GameObject *collector)
{
    globalVars::globalFreezeTimeout = 10;
    globalVars::globalTimeFreeze = true;
    globalVars::globalFreezeChronometer.reset(true);

    Collectable::onCollected(collector);
    return true;
}

HelmetCollectable::HelmetCollectable(GameObject *parent)
: Collectable(parent)
{}

bool HelmetCollectable::onCollected(GameObject *collector)
{
    PlayerController *controller = collector->getComponent<PlayerController>();
    controller->setTemporaryInvincibility(7500);

    Collectable::onCollected(collector);
    return true;
}

TankCollectable::TankCollectable(GameObject *parent)
: Collectable(parent)
{}

bool TankCollectable::onCollected(GameObject *collector)
{
    assert(collector != nullptr);
    assert(collector->isFlagSet(GameObject::Player));

    auto spawnerObject = collector->getParentObject();
    assert(spawnerObject != nullptr);

    auto spawnerController = spawnerObject->getComponent<PlayerSpawnController>();
    assert(spawnerController != nullptr);

    spawnerController->appendLife();

    Collectable::onCollected(collector);
    return true;
}

XpCollectable::XpCollectable(GameObject *parent, int value)
: Collectable(parent), _value(value)
{}

bool XpCollectable::onCollected(GameObject *collector)
{
    PlayerController *controller = collector->getComponent<PlayerController>();
    controller->addXP(_value);

    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::xpCollect, true);
    return true;
}

AmmoCollectable::AmmoCollectable(GameObject *parent)
: Collectable(parent)
{}

bool AmmoCollectable::onCollected(GameObject *collector)
{
    assert(collector->isFlagSet(GameObject::Player));
    auto shootable = collector->getComponent<Shootable>();
    shootable->addTempBullets(_amount);
    return true;
}

RepairCollectable::RepairCollectable(GameObject *parent)
: Collectable(parent)
{}

bool RepairCollectable::onCollected(GameObject *collector)
{
    assert(collector->isFlagSet(GameObject::Player));
    auto damageable = collector->getComponent<Damageable>();
    assert(damageable != nullptr);
    if (damageable->defence() < damageable->maxDefence()) {
        damageable->restoreDefence();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusCollect, true);
        auto controller = collector->getComponent<PlayerController>();
        controller->updateAppearance();
        return true;
    } else {
        return false;
    }

}

#include "PlayerUpgrade.h"

SkullCollectable::SkullCollectable(GameObject *parent)
: Collectable(parent)
{}

SkullCollectable::~SkullCollectable()
{
    if (!_gotCollected) {
        for (auto it = playerUpgrades.begin(); it != playerUpgrades.end(); ) {
            PlayerUpgrade *obj = (*it);
            it = playerUpgrades.erase(it);
            delete obj;
        }
    }
}

bool SkullCollectable::onCollected(GameObject *collector)
{
    assert(collector != nullptr);
    assert(collector->isFlagSet(GameObject::Player));

    collector->getComponent<PlayerController>()->restoreLevelAndUpgrades(this);
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusCollect, true);
    _gotCollected = true;
    return true;
}