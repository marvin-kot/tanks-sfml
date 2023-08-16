#include "Damageable.h"
#include "EagleController.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "SpriteRenderer.h"

EagleController::EagleController(GameObject *obj)
: Controller(obj, 0)
{}

EagleController::~EagleController()
{
    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
        delete obj;
    }
}

void EagleController::update()
{
    checkForGamePause();
    if (_pause) return;
}

void EagleController::upgrade(PlayerUpgrade *upgrade)
{
    assert(upgrade != nullptr);

    auto tp = upgrade->type();
    int lvl = hasLevelOfUpgrade(tp);
    // TODO magic names
    if (lvl > -1 && lvl < 3) {
        _collectedUpgrades[tp]->increaseLevel();
    } else {
        _collectedUpgrades[tp] = upgrade;
    }
}

void EagleController::applyUpgrades()
{
    // re-new all bonuses (like, armor protection etc)
    for (auto it : _collectedUpgrades) {
        it.second->onCollect(_gameObject);
    }

    updateAppearance();
}

void EagleController::updateAppearance()
{
    SpriteRenderer *renderer = _gameObject->getComponent<SpriteRenderer>();
    assert(renderer != nullptr);
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);

    switch (damageable->defence()) {
        case 0:
            renderer->setSpriteSheetOffset(0, 0);
            break;
        case 1:
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 2:
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 3:
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 4:
            renderer->setSpriteSheetOffset(0, 32);
            break;
    }

    renderer->showAnimationFrame(0);
}





int EagleController::hasLevelOfUpgrade(PlayerUpgrade::UpgradeType type) const
{
    if (_collectedUpgrades.find(type) == _collectedUpgrades.end())
        return -1;
    else
        return _collectedUpgrades.at(type)->currentLevel();
}

int EagleController::numberOfUpgrades() const
{
    return _collectedUpgrades.size();
}

PlayerUpgrade *EagleController::getUpgrade(int index) const
{
    assert(index < _collectedUpgrades.size());

    int i = 0;
    for (auto u : _collectedUpgrades) {
        if (i == index)
            return u.second;
        i++;
    }

    return nullptr;
}