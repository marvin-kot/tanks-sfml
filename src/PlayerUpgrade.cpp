#include "AssetManager.h"
#include "Damageable.h"
#include "EagleController.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "PersistentGameData.h"
#include "PlayerUpgrade.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "GlobalConst.h"
#include "Utils.h"

#include <random>

using UpgradeType = PlayerUpgrade::UpgradeType;

std::vector<PlayerUpgrade *> PlayerUpgrade::currentThreeRandomUpgrades = {};
std::vector<PlayerUpgrade *> PlayerUpgrade::availablePerkObjects = {};
std::unordered_set<UpgradeType> PlayerUpgrade::playerOwnedPerks = {};


std::vector<UpgradeType> PlayerUpgrade::availableTypes = {
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
        RebuildEagleWalls,
        RepairWalls,
        BaseArmor,
        BaseInvincibility,
        DepositXP
};

void PlayerUpgrade::removeFromAvailable(PlayerUpgrade::UpgradeType t)
{
    std::remove(availableTypes.begin(), availableTypes.end(), t);
}

const std::unordered_set<UpgradeType> tankUpgradeTypes = {
    UpgradeType::FastBullets,
    UpgradeType::MoreBullets,
    UpgradeType::TankSpeed,
    UpgradeType::PowerBullets,
    UpgradeType::TankArmor,
    UpgradeType::XpAttractor,
    PlayerUpgrade::BonusEffectiveness,
    PlayerUpgrade::Rocket,
    PlayerUpgrade::PiercingBullets,
    PlayerUpgrade::BulletTank
};

const std::unordered_set<UpgradeType> baseUpgradeTypes = {
    UpgradeType::BaseArmor,
    UpgradeType::BaseInvincibility,
    UpgradeType::RepairWalls,
    UpgradeType::BaseRestoreHP,
    UpgradeType::ReflectBullets,
    UpgradeType::PhoenixBase,
};

const std::unordered_set<UpgradeType> oneTimeBonusTypes = {
    UpgradeType::FreezeEnemies,
    UpgradeType::InstantKillEnemies,
    UpgradeType::TempTankInvincibility,
    UpgradeType::AdditionalLife,
    UpgradeType::RandomWalls,
    UpgradeType::InstantKillBaseArea,
    UpgradeType::CollectAllFreeXP,
    UpgradeType::RebuildEagleWalls,
    UpgradeType::DepositXP
};

const std::unordered_set<UpgradeType> perkTypes = {
    PlayerUpgrade::KillAllOnDeath,
    PlayerUpgrade::SacrificeLifeForBase,
    PlayerUpgrade::PlusLifeOnStart,
    PlayerUpgrade::XpIncreaser
};

const std::map<UpgradeType, std::pair<UpgradeType, int>> upgradeDepencies = {};

const std::map<UpgradeType, int> upgradeCap = {
    {UpgradeType::BonusEffectiveness, 3},
    {UpgradeType::RepairWalls, 3},
    {UpgradeType::BaseInvincibility, 4},
    {UpgradeType::MoreBullets,  4},
    {UpgradeType::FastBullets,  3},
    {UpgradeType::TankArmor,  4},
    {UpgradeType::TankSpeed,  4},
    {UpgradeType::PowerBullets,  3},
    {UpgradeType::BaseArmor,  4},
    {UpgradeType::XpIncreaser,  1},
    {UpgradeType::PiercingBullets,  1},
    {UpgradeType::KillAllOnDeath,  1},
    {UpgradeType::BulletTank,  1}
};


PlayerUpgrade::PlayerUpgrade(int level) : _currentLevel(level)
{
}

std::string PlayerUpgrade::name() const
{
    using namespace std;

    string levelStr;

    if (!_synergic) {
        switch (_category)
        {
            case OneTimeBonus:
                levelStr = "\n{Instant bonus}";
                break;
            case TankUpgrade:
                levelStr = "\n{Tank upgrade lvl." + to_string(_currentLevel+1) + "}";
                break;
            case BaseUpgrade:
                levelStr = "\n{Base upgrade lvl." + to_string(_currentLevel+1) + "}";
                break;
        }
    } else {
        levelStr = "\n{Super Upgrade}";
    }

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


static bool isUpgradedToMax(PlayerController *controller, PlayerUpgrade::UpgradeType t)
{
    int cap = upgradeCap.contains(t) ? upgradeCap.at(t) : 4;
    return controller->hasLevelOfUpgrade(t) >= (cap - 1);
}

static bool isUpgradedToMax(EagleController *controller, PlayerUpgrade::UpgradeType t)
{
    int cap = upgradeCap.contains(t) ? upgradeCap.at(t) : 4;
    return controller->hasLevelOfUpgrade(t) >= (cap - 1);
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

    std::unordered_set<UpgradeType> alreadyGenerated;

    std::vector<UpgradeType> availableTypesLocal = availableTypes;

    // remove not needed upgrades
    // TODO: find better solution
    if (isUpgradedToMax(controller, PiercingBullets)) {
        std::remove(availableTypesLocal.begin(), availableTypesLocal.end(), FastBullets);
        std::remove(availableTypesLocal.begin(), availableTypesLocal.end(), PowerBullets);
    }

    if (isUpgradedToMax(controller, BulletTank)) {
        std::remove(availableTypesLocal.begin(), availableTypesLocal.end(), TankSpeed);
        std::remove(availableTypesLocal.begin(), availableTypesLocal.end(), TankArmor);
    }

    std::uniform_int_distribution<int> distr(0, availableTypesLocal.size()-1);

    // determine cases where special non-upgrades have to be generated
    std::stack<UpgradeType> mandatoryUpgrades;
    if (isUpgradedToMax(controller, FastBullets) && isUpgradedToMax(controller, PowerBullets))
        mandatoryUpgrades.push(PiercingBullets);

    if (isUpgradedToMax(controller, TankSpeed) && isUpgradedToMax(controller, TankArmor))
        mandatoryUpgrades.push(BulletTank);


    for (int i=0; i<3; i++) {

        UpgradeType newType = None;
        do {
            int index = distr(Utils::generator);

            UpgradeType t = None;
            if (mandatoryUpgrades.empty()) {
                t = availableTypesLocal[index];
            } else {
                t = mandatoryUpgrades.top();
                mandatoryUpgrades.pop();
            }

            if (alreadyGenerated.find(t) != alreadyGenerated.end())
                continue;

            // check if player reached the limit of this upgrade
            if (tankUpgradeTypes.contains(t) && isUpgradedToMax(controller, t))
                continue;
            if (baseUpgradeTypes.contains(t) && isUpgradedToMax(eagleController, t))
                continue;

            // check dependencies
            if (upgradeDepencies.contains(t)) {
                if (tankUpgradeTypes.contains(t) && controller->hasLevelOfUpgrade(upgradeDepencies.at(t).first) < upgradeDepencies.at(t).second)
                    continue;
                else if (baseUpgradeTypes.contains(t) && eagleController->hasLevelOfUpgrade(upgradeDepencies.at(t).first) < upgradeDepencies.at(t).second)
                    continue;
            }

            // if this is not an instant bonus + player already have max number of upgrades -> generate another one
            if (!oneTimeBonusTypes.contains(newType) && controller->hasLevelOfUpgrade(t) == -1 && eagleController->hasLevelOfUpgrade(t) == -1
                    && controller->numberOfUpgrades() + eagleController->numberOfUpgrades() >= globalConst::PlayerUpgradesLimit)
                continue;

             if (t == RebuildEagleWalls && ObjectsPool::getEagleWalls().size() == globalTypes::EagleWallDirection::MaxDirection)
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

void PlayerUpgrade::generatePerks()
{
    if (!availablePerkObjects.empty()) return;

    for (auto perkType : perkTypes) {
        availablePerkObjects.push_back(createUpgrade(perkType, 0));
    }
}

void PlayerUpgrade::deletePerks()
{
    if (availablePerkObjects.empty()) return;

    while (!availablePerkObjects.empty()) {
        auto it = std::prev(availablePerkObjects.end());
        PlayerUpgrade *obj = *it;
        availablePerkObjects.erase(it);
        delete obj;
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
        case RebuildEagleWalls:
            newUpgrade = new RebuildEagleWallsBonus(level);
            break;
        case DepositXP:
            newUpgrade = new DepositXpBonus(level);
            break;
        case RepairWalls:
            newUpgrade = new RebuildEagleWallsOnLevelup(level);
            break;
        case BaseArmor:
            newUpgrade = new BaseArmorUpgrade(level);
            break;
        case BaseInvincibility:
            newUpgrade = new EagleInvincibilityAfterDamage(level);
            break;
        case XpIncreaser:
            newUpgrade = new XpModifierUpgrade(level);
            break;
        case PiercingBullets:
            newUpgrade = new PiercingBulletsUpgrade(level);
            break;
        case KillAllOnDeath:
            newUpgrade = new KillAllOnTankDeathUpgrade(level);
            break;
        case BulletTank:
            newUpgrade = new BulletTankUpgrade(level);
        case SacrificeLifeForBase:
            newUpgrade = new SacrificeLifeForBaseUpgrade(level);
            break;
        case PlusLifeOnStart:
            newUpgrade = new TankAdditionalLifePerk();
            break;
    }

    return newUpgrade;
}

bool PlayerUpgrade::firstApplication() {
    if (!_applied) {
        _applied = true;
        return true;
    } else return false;
}

void PlayerUpgrade::reset()
{
    _applied = false;
}

BonusEffectivenessIncreaser::BonusEffectivenessIncreaser(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = BonusEffectiveness;
    _name = "Effective electronics";

    _effects.push_back("Improves effect of\nevery instant bonus");
    _effects.push_back("Greatly improves effect\nof every instant bonus");
    _effects.push_back("Maxizes effect of\nevery instant bonus");

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

    _timeoutBasedOnLevel = { 10, 15, 20, 25 };
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

    _percentBasedOnLevel = { 33, 50, 75, 100 };
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
    _name = "Power shield";

    _timeBasedOnLevel = { 10, 15, 20, 25 };
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
    controller->setTemporaryInvincibility(_timeBasedOnLevel[_currentLevel] * 100);
}

///////////////
TankAdditionalLifeBonus::TankAdditionalLifeBonus() : PlayerUpgrade(0) {}

TankAdditionalLifeBonus::TankAdditionalLifeBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = PlayerUpgrade::AdditionalLife;
    _name = "Spare tank";

    _numberBasedOnLevel = { 1, 1, 2, 3 };
    for (auto time : _numberBasedOnLevel) {
        std::string ending = time>1 ? " lives" : " life";
        _effects.push_back("+" + std::to_string(time) + ending);
    }

    _iconRect = AssetManager::instance().getAnimationFrame("plusCollectable", "default", 0).rect;

}

void TankAdditionalLifeBonus::onCollect(GameObject *collector)
{
    if (!firstApplication()) return;

    globalVars::player1Lives++;

    /*GameObject *playerSpawnerObject;
    if (collector->isFlagSet(GameObject::Player))
        playerSpawnerObject = collector->getParentObject();
    else if (collector->isFlagSet(GameObject::PlayerSpawner))
        playerSpawnerObject = collector;
    else
        playerSpawnerObject = ObjectsPool::playerSpawnerObject;

    assert(playerSpawnerObject != nullptr);
    assert(playerSpawnerObject->isFlagSet(GameObject::PlayerSpawner));

    auto spawnerController = playerSpawnerObject->getComponent<PlayerSpawnController>();
    assert(spawnerController != nullptr);

    spawnerController->appendLife();*/
}

TankAdditionalLifePerk::TankAdditionalLifePerk()
{
    _currentLevel = 0;
    _category = PlayerUpgrade::Perk;
    _price = 0;
    _type = PlayerUpgrade::PlusLifeOnStart;
    _name = "Spare tank";

    _effects.push_back("Start with +1 life");

    _iconRect = AssetManager::instance().getAnimationFrame("plusCollectable", "default", 0).rect;
}


/////////////
RebuildEagleWallsBonus::RebuildEagleWallsBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = PlayerUpgrade::RebuildEagleWalls;
    _name = "Repair works";

     _numberBasedOnLevel = { 0, 0, 0, 0, 0 };
    for (auto time : _numberBasedOnLevel) {
        _effects.push_back("Restore the base walls. Once");
    }

    _iconRect = AssetManager::instance().getAnimationFrame("shovelCollectable", "default", 0).rect;

}

void RebuildEagleWallsBonus::onCollect(GameObject *)
{

    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);
    eagleController->fastRepairWalls(100);
}

DepositXpBonus::DepositXpBonus(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::OneTimeBonus;
    _type = PlayerUpgrade::DepositXP;
    _name = "Knowledge is money";

    _effects.push_back("\nturn all your XP to $\nand put it on deposit\n$ can be used in shop later\nall xp and tank upgrades will be reset");


    _iconRect = AssetManager::instance().getAnimationFrame("depositXpCollectable", "default", 0).rect;

}

void DepositXpBonus::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    PlayerController *controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    int xpToSave = controller->xp();
    controller->resetXP();

    PersistentGameData::instance().addToXpDeposit(xpToSave);
}

///////////////

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
///////////////

FasterBulletUpgrade::FasterBulletUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = FastBullets;
    _name = "Fast delivery";

    _percentBasedOnLevel = { 30, 60, 80 };
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

    const int newBulletSpeed = globalConst::DefaultPlayerBulletSpeed + (globalConst::DefaultPlayerBulletSpeed * percent / 100);
    shootable->setBulletSpeed(newBulletSpeed);
}


///////////////
MoreBulletsUpgrade::MoreBulletsUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = MoreBullets;
    _name = "Productive turret";

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
    _name = "Coat of steel";

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
    damageable->setDefence(globalConst::DefaultPlayerProtection + number);
}

/////////////

FasterTankUpgrade::FasterTankUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = TankSpeed;
    _name = "Horse powers";
    _percentBasedOnLevel = { 20, 40, 60, 80 };
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
    _name = "Power bullets";

    _numberBasedOnLevel = { 1, 2, 3};
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


PiercingBulletsUpgrade::PiercingBulletsUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type = PiercingBullets;
    _name = "Piercing bullets";

    _effects.push_back("Replaces Power Bullets and Fast Delivery\nBullets fly through enemies,\nlosing 1 damage per enemy.\nIgnores enemy bullets");

    _iconRect = AssetManager::instance().getAnimationFrame("piercingCollectable", "default", 0).rect;
}

void PiercingBulletsUpgrade::onCollect(GameObject *target)
{
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Player));
    auto shootable = target->getComponent<Shootable>();
    shootable->setPiercing(true);

    // TODO: thing how to avoid magic numbers
    int newDamage = globalConst::DefaultDamage + 3;
    shootable->setDamage(newDamage);

    const int newBulletSpeed = globalConst::DefaultPlayerBulletSpeed + (globalConst::DefaultPlayerBulletSpeed * 80 / 100);
    shootable->setBulletSpeed(newBulletSpeed);

}

/////////////


BulletTankUpgrade::BulletTankUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type  = BulletTank;
    _name = "Death machine";

    _effects.push_back("Replaces [Speed] and [Armor] upgrades\nAfter moving straight for 1 sec,\nTank becomes an invulnerable death\nmachine, until it stops");

    _iconRect = AssetManager::instance().getAnimationFrame("bulletTankCollectable", "default", 0).rect;
}

void BulletTankUpgrade::onCollect(GameObject *)
{
    // nothing is needed except the fact of presense of this effect
}


/////////////


XpModifierUpgrade::XpModifierUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _price = 0;
    _type = XpIncreaser;
    _name = "War machine learning";

    _numberBasedOnLevel = { 25 };
    for (auto number : _numberBasedOnLevel)
        _effects.push_back("Get +" + std::to_string(number) + "\% more experience");

    _iconRect = AssetManager::instance().getAnimationFrame("xpCollectable", "default", 0).rect;
}

void XpModifierUpgrade::onCollect(GameObject *target)
{
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Player));
    auto controller = target->getComponent<PlayerController>();
    controller->setXpModifier(_numberBasedOnLevel[_currentLevel]);
}

//////////




KillAllOnTankDeathUpgrade::KillAllOnTankDeathUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _price = 0;
    _type = KillAllOnDeath;
    _name = "Atomic Core";

    _effects.push_back("When your tank dies,\nall the enemies will perish");

    _iconRect = AssetManager::instance().getAnimationFrame("redTankCollectable", "default", 0).rect;
}

void KillAllOnTankDeathUpgrade::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}


SacrificeLifeForBaseUpgrade::SacrificeLifeForBaseUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _price = 0;
    _type = SacrificeLifeForBase;
    _name = "Sacrifice";

    _effects.push_back("Sacrifice 1 life once to revive\nthe base and repair its walls");

    _iconRect = AssetManager::instance().getAnimationFrame("redEagleCollectable", "default", 0).rect;
}

void SacrificeLifeForBaseUpgrade::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}