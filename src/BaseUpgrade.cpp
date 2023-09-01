#include "AssetManager.h"
#include "BaseUpgrade.h"
#include "Damageable.h"
#include "GameObject.h"
#include "EagleController.h"
#include "ObjectsPool.h"

#include <cassert>



RebuildEagleWallsOnLevelup::RebuildEagleWallsOnLevelup(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::BaseUpgrade;
    _type = PlayerUpgrade::RepairWalls;
    _name = "Inspired builders";

    _numberBasedOnLevel = { 4, 2, 1 };
    std::vector<std::string> suffix = {"4th ", "2nd ", ""};
    std::string prefix = level == 0 ? "now and " : "";
    int i=0;
    for (auto num : _numberBasedOnLevel) {
        _effects.push_back("Base walls will be rebuilt\n" + prefix + "on every " + suffix[i] + "level up");
        i++;
    }

    _iconRect = AssetManager::instance().getAnimationFrame("shovelCollectable", "default", 0).rect;
}

void RebuildEagleWallsOnLevelup::onCollect(GameObject *)
{

    if (levelupCounter % _numberBasedOnLevel[_currentLevel] == 0) {
        auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
        assert(eagleController != nullptr);
        eagleController->fastRepairWalls(100);
    }

    levelupCounter++;
}

///////////////

EagleInvincibilityAfterDamage::EagleInvincibilityAfterDamage(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::BaseUpgrade;
    _type = PlayerUpgrade::BaseInvincibility;
    _name = "Emergency cupola";

    // requires any level of armor
    _dependencies[BaseArmor] = 0;

    _timeBasedOnLevel = { 5, 10, 15, 20 };
    //std::vector<std::string> suffix = {"th", "rd", "nd", "st"};
    for (auto time : _timeBasedOnLevel) {
        _effects.push_back("After getting damaged,\nthe base will become\ninvincible for " + std::to_string(time) + " sec");
    }

    _iconRect = AssetManager::instance().getAnimationFrame("eagleCloudCollectable", "default", 0).rect;

}

void EagleInvincibilityAfterDamage::onCollect(GameObject *)
{
    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);
    eagleController->setTempInvincibilityAfterDamage(_timeBasedOnLevel[_currentLevel] * 1000);
}

BaseArmorUpgrade::BaseArmorUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::BaseUpgrade;
    _type = BaseArmor;
    _name = "Thicker feathers";

    _numberBasedOnLevel = { 1, 2, 3, 4 };
    for (auto number : _numberBasedOnLevel)
        _effects.push_back("Base protection +" + std::to_string(number));

    _iconRect = AssetManager::instance().getAnimationFrame("eagleCollectable", "default", 0).rect;
}

void BaseArmorUpgrade::onCollect(GameObject *target)
{
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Eagle));

    auto damageable = target->getComponent<Damageable>();
    assert(damageable != nullptr);
    assert(_currentLevel < _numberBasedOnLevel.size());
    int newProtection = globalConst::DefaultBaseProtection + _numberBasedOnLevel[_currentLevel];
    damageable->setDefence(newProtection);
}

/////////////

BaseRepairAfterDamageUpgrade::BaseRepairAfterDamageUpgrade()
: PlayerUpgrade(0) {
     _category = PlayerUpgrade::BaseUpgrade;
    _type = BaseRestoreOnDamage;
    _name = "Extra Repair Squad";

    _effects.push_back("Replaces [inspired builders]\nand[emergency cupola]\nBase will rebuild walls\non getting damage");

    _iconRect = AssetManager::instance().getAnimationFrame("eagleBrickCollectable", "default", 0).rect;
}

void BaseRepairAfterDamageUpgrade::onCollect(GameObject *)
{
    assert(ObjectsPool::eagleObject != nullptr);
    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);

    eagleController->fastRepairWalls(100);
    eagleController->setTempInvincibilityAfterDamage(10000);
}