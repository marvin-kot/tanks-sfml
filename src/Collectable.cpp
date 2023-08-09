#include "Collectable.h"

#include "GameObject.h"
#include "ObjectsPool.h"
#include "SoundPlayer.h"
#include "SpriteRenderer.h"
#include "Controller.h"
#include "GlobalConst.h"

Collectable::Collectable(GameObject *parent)
: _gameObject(parent)
{}

void Collectable::onCollected(GameObject *collector)
{
    (void *)collector;
    SoundPlayer::instance().playBonusCollectSound();
}

///////

GrenadeCollectable::GrenadeCollectable(GameObject *parent)
: Collectable(parent)
{}

void GrenadeCollectable::onCollected(GameObject *collector)
{
    // kill all enemy tanks in a moment
    std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank"});
    std::for_each(objectsToKill.cbegin(), objectsToKill.cend(), [](GameObject *obj) { obj->_deleteme = true; });

    Collectable::onCollected(collector);
}

//////

StarCollectable::StarCollectable(GameObject *parent)
: Collectable(parent)
{}


void StarCollectable::onCollected(GameObject *collector)
{
    assert(collector->isFlagSet(GameObject::Player));

    PlayerController *controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    controller->increasePowerLevel(true);


    Collectable::onCollected(collector);
}

//////

TimerCollectable::TimerCollectable(GameObject *parent)
: Collectable(parent)
{}

void TimerCollectable::onCollected(GameObject *collector)
{
    globalVars::globalTimeFreeze = true;
    globalVars::globalFreezeClock.restart();

    Collectable::onCollected(collector);
}


//////

HelmetCollectable::HelmetCollectable(GameObject *parent)
: Collectable(parent)
{}

void HelmetCollectable::onCollected(GameObject *collector)
{
    PlayerController *controller = collector->getComponent<PlayerController>();
    controller->setTemporaryInvincibility(10);
}


