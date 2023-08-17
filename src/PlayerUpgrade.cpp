#include "AssetManager.h"
#include "Damageable.h"
#include <EagleController.h>
#include "GameObject.h"
#include "ObjectsPool.h"
#include "PlayerUpgrade.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "GlobalConst.h"
#include "Utils.h"

#include <unordered_set>
#include <random>

std::vector<PlayerUpgrade *> PlayerUpgrade::currentThreeRandomUpgrades = {};

const std::unordered_set<PlayerUpgrade::UpgradeType> playerPlayerUpgradeTypes = {
    PlayerUpgrade::FastBullets,
    PlayerUpgrade::MoreBullets,
    PlayerUpgrade::TankSpeed,
    PlayerUpgrade::PowerBullets,
    PlayerUpgrade::TankArmor,
    PlayerUpgrade::XpAttractor,
    PlayerUpgrade::BonusEffectiveness,
    PlayerUpgrade::Rocket
};

const std::unordered_set<PlayerUpgrade::UpgradeType> baseUpgradeTypes = {
    PlayerUpgrade::BaseArmor,
    PlayerUpgrade::RepairWalls,
    PlayerUpgrade::BaseRestoreHP,
    PlayerUpgrade::ReflectBullets,
    PlayerUpgrade::PhoenixBase,
};

const std::unordered_set<PlayerUpgrade::UpgradeType> oneTimeBonusTypes = {
    PlayerUpgrade::FreezeEnemies,
    PlayerUpgrade::InstantKillEnemies,
    PlayerUpgrade::TempTankInvincibility,
    PlayerUpgrade::AdditionalLife,
    PlayerUpgrade::RandomWalls,
    PlayerUpgrade::InstantKillBaseArea,
    PlayerUpgrade::CollectAllFreeXP
};

PlayerUpgrade::PlayerUpgrade(int level) : _currentLevel(level)
{
    _dependency.first = None;
    _dependency.second = 0;
}

std::string PlayerUpgrade::name() const
{
    using namespace std;
    string levelStr = _category != OneTimeBonus ? string("\n{lvl." + to_string(_currentLevel+1) + "}") : string();
    return _name + levelStr;
}

std::string PlayerUpgrade::currentEffectDescription() const
{
    if (_effects.size() > _currentLevel)
        return _effects[_currentLevel];
    else if (!_effects.empty())
        return _effects[0];
    else
        return "Unknown effect";
}

void PlayerUpgrade::generateThreeRandomUpgradesForPlayer(GameObject *playerObj)
{
    Logger::instance() << "generate three random upgrades\n";
    assert(playerObj != nullptr);

    auto controller = playerObj->getComponent<PlayerController>();
    assert(controller != nullptr);

    assert(ObjectsPool::eagleObject != nullptr);
    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);

    currentThreeRandomUpgrades.clear();

    std::vector<UpgradeType> availableTypes = {
        FastBullets,
        MoreBullets,
        TankSpeed,
        PowerBullets,
        TankArmor,
        BonusEffectiveness,
        FreezeEnemies,
        InstantKillEnemies,
        TempTankInvincibility,
        AdditionalLife,
        BaseArmor
    };

    std::unordered_set<UpgradeType> alreadyGenerated;

    std::uniform_int_distribution<int> distr(0, availableTypes.size()-1);

    for (int i=0; i<3; i++) {

        UpgradeType newType = None;
        do {
            int index = distr(Utils::generator);
            Logger::instance() << "index is" << index << "\n";
            UpgradeType t = availableTypes[index];
            if (alreadyGenerated.find(t) != alreadyGenerated.end())
                continue;

            if (controller->hasLevelOfUpgrade(t) > 2)
                continue;

            // if this is not an instant bonus + player already have max number of upgrades -> generare another one
            if (!oneTimeBonusTypes.contains(newType) && controller->hasLevelOfUpgrade(t) == -1
                    && controller->numberOfUpgrades() + eagleController->numberOfUpgrades() >= globalConst::PlayerUpgradesLimit)
                continue;

            newType = t;
            alreadyGenerated.insert(t);

        } while (newType == None);

        int oldLevel = controller->hasLevelOfUpgrade(newType);

        // if is one time bonus
        if (oneTimeBonusTypes.contains(newType)) {
            // and player has effectiveness modifier
            oldLevel = controller->hasLevelOfUpgrade(BonusEffectiveness);
        }

        if (baseUpgradeTypes.contains(newType))
            oldLevel = eagleController->hasLevelOfUpgrade(newType);

        currentThreeRandomUpgrades.push_back(createUpgrade(newType, oldLevel+1));
    }
}

PlayerUpgrade *PlayerUpgrade::createUpgrade(UpgradeType type, int level)
{
    PlayerUpgrade *newUpgrade = nullptr;
    switch (type) {
        case FastBullets:
            newUpgrade = new FasterBulletUpgrade(level);
            break;
        case MoreBullets:
            newUpgrade = new MoreBulletsUpgrade(level);
            break;
        case TankSpeed:
            newUpgrade = new FasterTankUpgrade(level);
            break;
        case PowerBullets:
            newUpgrade = new PowerBulletUpgrade(level);
            break;
        case TankArmor:
            newUpgrade = new ArmorUpgrade(level);
            break;
        case BonusEffectiveness:
            newUpgrade = new BonusEffectivenessIncreaser(level);
            break;
        case FreezeEnemies:
            newUpgrade = new FreezeEnemiesBonus(level);
            break;
        case InstantKillEnemies:
            newUpgrade = new KillEnemiesBonus(level);
            break;
        case TempTankInvincibility:
            newUpgrade = new TankInvincibilityBonus(level);
            break;
        case AdditionalLife:
            newUpgrade = new TankAdditionalLifeBonus(level);
            break;
        case BaseArmor:
            newUpgrade = new BaseArmorUpgrade(level);
            break;
    }

    return newUpgrade;
}

BonusEffectivenessIncreaser::BonusEffectivenessIncreaser(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = BonusEffectiveness;
    _name = "Modern machinery";

    _effects.push_back("Improves an effect of\nevery one-time bonus");
    _effects.push_back("Severally improves an\neffect of every one-time bonus");
    _effects.push_back("Greatly improves an effect\nof every one-time bonus");
    _effects.push_back("Maxize an effect of\nevery one-time bonus");

    _iconRect = AssetManager::instance().getAnimationFrame("starCollectable", "default", 0).rect;
}

void BonusEffectivenessIncreaser::onCollect(GameObject *)
{}

////////////////

FreezeEnemiesBonus::FreezeEnemiesBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = FreezeEnemies;
    _name = "Frostbite";

    _timeoutBasedOnLevel = { 10, 13, 15, 20, 24 };
    for (auto time : _timeoutBasedOnLevel)
        _effects.push_back("Freezes all enemy tanks\nfor " + std::to_string(time) + " sec");

    _iconRect = AssetManager::instance().getAnimationFrame("timerCollectable", "default", 0).rect;
}

void FreezeEnemiesBonus::onCollect(GameObject *)
{
    assert(_currentLevel < _timeoutBasedOnLevel.size());
    globalVars::globalFreezeTimeout = _timeoutBasedOnLevel[_currentLevel];
    globalVars::globalTimeFreeze = true;
    globalVars::globalFreezeChronometer.reset(true);
}

///////////////

KillEnemiesBonus::KillEnemiesBonus(int level)
: PlayerUpgrade(level)
, distribution(1, 100)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = InstantKillEnemies;
    _name = "Airstrike";

    _percentBasedOnLevel = { 33, 50, 66, 90, 100 };
    for (auto percent : _percentBasedOnLevel)
        _effects.push_back("Every enemy tank\nwill be destroyed\nwith " + std::to_string(percent) + "\% chance");

    _iconRect = AssetManager::instance().getAnimationFrame("grenadeCollectable", "default", 0).rect;
}

void KillEnemiesBonus::onCollect(GameObject *)
{
    assert(_currentLevel < _percentBasedOnLevel.size());
    // kill all enemy tanks (with probability)in a moment

    std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank"});
    std::for_each(objectsToKill.cbegin(), objectsToKill.cend(), [&](GameObject *obj) {
        int rnd = distribution(Utils::generator);
        if (rnd <= _percentBasedOnLevel[_currentLevel])
            obj->markForDeletion();
    });
}
///////////////

TankInvincibilityBonus::TankInvincibilityBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = PlayerUpgrade::TempTankInvincibility;
    _name = "Divine cloud";

    _timeBasedOnLevel = { 10, 15, 20, 25, 30 };
    for (auto time : _timeBasedOnLevel)
        _effects.push_back("Player tank becomes\ninvincible for " + std::to_string(time) + " sec");

    _iconRect = AssetManager::instance().getAnimationFrame("cloudCollectable", "default", 0).rect;

}

void TankInvincibilityBonus::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    PlayerController *controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    assert(_currentLevel < _timeBasedOnLevel.size());
    controller->setTemporaryInvincibility(_timeBasedOnLevel[_currentLevel]);
}

///////////////

TankAdditionalLifeBonus::TankAdditionalLifeBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = PlayerUpgrade::AdditionalLife;
    _name = "Spare tank";

    _numberBasedOnLevel = { 1, 1, 1, 2, 2 };
    for (auto time : _numberBasedOnLevel) {
        std::string ending = time>1 ? " lives" : " life";
        _effects.push_back("+" + std::to_string(time) + ending);
    }

    _iconRect = AssetManager::instance().getAnimationFrame("plusCollectable", "default", 0).rect;

}

void TankAdditionalLifeBonus::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    assert(collector->isFlagSet(GameObject::Player));

    auto spawnerObject = collector->getParentObject();
    assert(spawnerObject != nullptr);
    assert(spawnerObject->isFlagSet(GameObject::PlayerSpawner));

    auto spawnerController = spawnerObject->getComponent<PlayerSpawnController>();
    assert(spawnerController != nullptr);

    spawnerController->appendLife();
}


///////////////

FasterBulletUpgrade::FasterBulletUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = FastBullets;
    _name = "Muzzle upgrade";

    _percentBasedOnLevel = { 30, 60, 80, 100 };
    for (auto percent : _percentBasedOnLevel)
        _effects.push_back("Bullet speed +" + std::to_string(percent) + "\%");

    _iconRect = AssetManager::instance().getAnimationFrame("pistolCollectable", "default", 0).rect;
}

void FasterBulletUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<PlayerShootable>();
    assert(shootable != nullptr);

    assert(_currentLevel < _percentBasedOnLevel.size());
    int percent = _percentBasedOnLevel[_currentLevel];

    const int newBulletSpeed = globalConst::DefaultBulletSpeed + (globalConst::DefaultBulletSpeed * percent / 100);
    shootable->setBulletSpeed(newBulletSpeed);
}


///////////////
MoreBulletsUpgrade::MoreBulletsUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = MoreBullets;
    _name = "Turret upgrade";

    _numberBasedOnLevel = { 1, 2, 3, 4 };
    for (auto number : _numberBasedOnLevel)
        _effects.push_back("+" + std::to_string(number) + " bullets at time");

    _iconRect = AssetManager::instance().getAnimationFrame("tankCollectable", "default", 0).rect;
}

void MoreBulletsUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<PlayerShootable>();
    assert(shootable != nullptr);
    assert(_currentLevel < _numberBasedOnLevel.size());

    shootable->resetLevel();

    for (int i=0; i<_numberBasedOnLevel[_currentLevel]; i++)
        shootable->increaseLevel();
}


///////////////

ArmorUpgrade::ArmorUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = TankArmor;
    _name = "Armor upgrade";

    _numberBasedOnLevel = { 1, 2, 3, 4 };
    for (auto number : _numberBasedOnLevel)
        _effects.push_back("Tank protection +" + std::to_string(number));

    _iconRect = AssetManager::instance().getAnimationFrame("helmetCollectable", "default", 0).rect;
}

void ArmorUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto damageable = collector->getComponent<Damageable>();
    assert(damageable != nullptr);

    assert(_currentLevel < _numberBasedOnLevel.size());
    int number = _numberBasedOnLevel[_currentLevel];
    damageable->setDefence(number);
}

/////////////

FasterTankUpgrade::FasterTankUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = TankSpeed;
    _name = "Caterpillar upgrade";
    _percentBasedOnLevel = { 20, 30, 40, 50 };
    for (auto percent : _percentBasedOnLevel)
        _effects.push_back("Tank speed +" + std::to_string(percent) + "\%");

    _iconRect = AssetManager::instance().getAnimationFrame("fastCaterpillarCollectable", "default", 0).rect;
}

void FasterTankUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    assert(_currentLevel < _percentBasedOnLevel.size());
    int percent = _percentBasedOnLevel[_currentLevel];
    const int newTankSpeed = globalConst::DefaultPlayerSpeed + globalConst::DefaultPlayerSpeed * percent / 100;
    controller->updateMoveSpeed(newTankSpeed);
}

///////////////

PowerBulletUpgrade::PowerBulletUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = PowerBullets;
    _name = "Ammunition upgrade";

    _numberBasedOnLevel = { 1, 2, 3, 4 };
    for (auto number : _numberBasedOnLevel)
        _effects.push_back("Bullet damage +" + std::to_string(number) + "");

    _iconRect = AssetManager::instance().getAnimationFrame("bulletCollectable", "default", 0).rect;
}

void PowerBulletUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<Shootable>();
    assert(shootable != nullptr);

    assert(_currentLevel < _numberBasedOnLevel.size());
    int newDamage = globalConst::DefaultDamage + _numberBasedOnLevel[_currentLevel];
    shootable->setDamage(newDamage);
}


///////////////

BaseArmorUpgrade::BaseArmorUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::BaseUpgrade;
    _type = BaseArmor;
    _name = "Base armor upgrade";

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
    int newProtection = _numberBasedOnLevel[_currentLevel];
    damageable->setDefence(newProtection);
}