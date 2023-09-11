#include "AssetManager.h"
#include "BaseUpgrade.h"
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
#include <format>

using UpgradeType = PlayerUpgrade::UpgradeType;

std::vector<PlayerUpgrade *> PlayerUpgrade::currentRandomUpgrades = {};
std::vector<PlayerUpgrade *> PlayerUpgrade::availablePerkObjects = {};
std::unordered_set<UpgradeType> PlayerUpgrade::playerOwnedPerks = {};

std::map<UpgradeType, int> PlayerUpgrade::perkPrices = {
    {UpgradeType::SacrificeLifeForBase, 20000},
    {UpgradeType::PlusLifeOnStart, 10000},
    {UpgradeType::XpIncreaser, 12000},
    {UpgradeType::KillAllOnDeath, 30000},
    {UpgradeType::FavoriteTank, 9000},
    {UpgradeType::FavoriteBase, 9000}
};

int PlayerUpgrade::minPerkPrice = 9000;
// ::Perk

std::vector<UpgradeType> PlayerUpgrade::availableTypes = {
        FastBullets,
        MoreBullets,
        TankSpeed,
        PowerBullets,
        TankArmor,
        FastReload,
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
    UpgradeType::FastReload,
    UpgradeType::PowerBullets,
    UpgradeType::TankArmor,
    PlayerUpgrade::BonusEffectiveness,
    PlayerUpgrade::Rocket,
    PlayerUpgrade::PiercingBullets,
    PlayerUpgrade::BulletTank,
    PlayerUpgrade::FourDirectionBullets,
    PlayerUpgrade::Rocket,
    PlayerUpgrade::MachineGun
};

const std::unordered_set<UpgradeType> baseUpgradeTypes = {
    UpgradeType::BaseArmor,
    UpgradeType::BaseInvincibility,
    UpgradeType::RepairWalls,
    UpgradeType::BaseRestoreHP,
    UpgradeType::ReflectBullets,
    UpgradeType::PhoenixBase,
    UpgradeType::BaseRestoreOnDamage
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
    PlayerUpgrade::XpIncreaser,
    PlayerUpgrade::FavoriteTank,
    PlayerUpgrade::FavoriteBase
};

const std::map<UpgradeType, std::pair<UpgradeType, int>> upgradeDepencies = {};

const std::map<UpgradeType, std::unordered_set<UpgradeType>> upgradeNegations = {
    {UpgradeType::FastBullets, {UpgradeType::PiercingBullets, UpgradeType::MachineGun}},
    {UpgradeType::PowerBullets, {UpgradeType::PiercingBullets, UpgradeType::Rocket, UpgradeType::FourDirectionBullets, UpgradeType::ReloadOnKill}},
    {UpgradeType::FastReload, {UpgradeType::ReloadOnKill}},
    {UpgradeType::TankArmor, {UpgradeType::BulletTank}},
    {UpgradeType::TankSpeed, {UpgradeType::BulletTank}},
    {UpgradeType::MoreBullets, {UpgradeType::FourDirectionBullets, UpgradeType::MachineGun}},
    {UpgradeType::BonusEffectiveness, {UpgradeType::Rocket}},
    {UpgradeType::RepairWalls, {UpgradeType::BaseRestoreOnDamage}},
    {UpgradeType::BaseInvincibility, {UpgradeType::BaseRestoreOnDamage}}
};

const std::map<UpgradeType, int> upgradeCap = {
    {UpgradeType::BonusEffectiveness, 3},
    {UpgradeType::RepairWalls, 3},
    {UpgradeType::BaseInvincibility, 4},
    {UpgradeType::MoreBullets,  4},
    {UpgradeType::FastBullets,  4},
    {UpgradeType::FastReload,  4},
    {UpgradeType::TankArmor,  4},
    {UpgradeType::TankSpeed,  4},
    {UpgradeType::PowerBullets,  3},
    {UpgradeType::BaseArmor,  4},
    {UpgradeType::XpIncreaser,  1},
    {UpgradeType::PiercingBullets,  1},
    {UpgradeType::KillAllOnDeath,  1},
    {UpgradeType::FavoriteTank,  1},
    {UpgradeType::FavoriteBase,  1},
    {UpgradeType::BulletTank,  1},
    {UpgradeType::FourDirectionBullets,  1},
    {UpgradeType::ReloadOnKill,  1},
    {UpgradeType::Rocket,  1},
    {UpgradeType::MachineGun,  1},
    {UpgradeType::BaseRestoreOnDamage,  1}
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

static bool isNotHavingUpgrade(PlayerController *pController, EagleController *eController, PlayerUpgrade::UpgradeType t)
{
    return pController->hasLevelOfUpgrade(t) == -1 && eController->hasLevelOfUpgrade(t) == -1;
}

std::vector<UpgradeType> PlayerUpgrade::fillLocalTypesList(PlayerController *pContr, EagleController *eContr)
{
    assert( pContr != nullptr);
    assert( eContr != nullptr);

    std::vector<UpgradeType> availableTypesLocal;

    bool upgradeLimitReached = pContr->numberOfUpgrades() + eContr->numberOfUpgrades() >= globalConst::PlayerUpgradesLimit;

    for (auto it = availableTypes.begin(); it != availableTypes.end(); ++it) {
        PlayerUpgrade::UpgradeType t = *it;

        // do not generate new upgrades if limit is reached
        if (upgradeLimitReached && !oneTimeBonusTypes.contains(t) && isNotHavingUpgrade(pContr, eContr, t))
                continue;

        // check for super upgrades which negate this upgrade
        if (upgradeNegations.contains(t)) {
            bool hasNegatingSuperUpgrade = false;
            for (PlayerUpgrade::UpgradeType superUp : upgradeNegations.at(t)) {
                if (isUpgradedToMax(pContr, superUp) || isUpgradedToMax(eContr, superUp)) {
                    hasNegatingSuperUpgrade = true;
                    break; // for
                }
            }
            if (hasNegatingSuperUpgrade) continue;
        }

        // nullify or increase chances of appearing based on owned upgrades
        if (tankUpgradeTypes.contains(t)) {
            int maxLevel = upgradeCap.contains(t) ? upgradeCap.at(t)-1 : 3;
            int level = pContr->hasLevelOfUpgrade(t);
            if (level>-1 && level < maxLevel)
                availableTypesLocal.push_back(t); // increase chance by adding to the list once more
            else if (level == maxLevel)
                continue; // do not add to list
        } else if (baseUpgradeTypes.contains(t)) {
            // NEW - make base upgrades more expensive by starting generating them only from 4th level
            if (pContr->level() < 4) continue;
            int maxLevel = upgradeCap.contains(t) ? upgradeCap.at(t)-1 : 3;
            int level = eContr->hasLevelOfUpgrade(t);
            if (level>-1 && level < maxLevel)
                availableTypesLocal.push_back(t); // increase chance by adding to the list once more
            else if (level == maxLevel)
                continue; // do not add to list
        }

        // increase chances based on perks
        if (tankUpgradeTypes.contains(t) && playerOwnedPerks.contains(FavoriteTank))
            availableTypesLocal.push_back(t); // add once more
        if (baseUpgradeTypes.contains(t) && playerOwnedPerks.contains(FavoriteBase))
            availableTypesLocal.push_back(t); // add once more

        // specific cases
        if (t == RebuildEagleWalls && ObjectsPool::getEagleWalls().size() >= globalTypes::EagleWallDirection::MaxDirection-1)
            continue;
        availableTypesLocal.push_back(t);
    }

    return availableTypesLocal;
}

void PlayerUpgrade::generateRandomUpgradesForPlayer(GameObject *playerObj)
{
    Logger::instance() << "generate four random upgrades\n";
    assert(playerObj != nullptr);

    auto controller = playerObj->getComponent<PlayerController>();
    assert(controller != nullptr);

    assert(ObjectsPool::eagleObject != nullptr);
    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);
    if (eagleController == nullptr) {
         Logger::instance() << "[ERROR]No eagle!\n";
        return;
    }

    currentRandomUpgrades.clear();

    std::vector<UpgradeType> availableTypesLocal = fillLocalTypesList(controller, eagleController);
    assert(availableTypesLocal.empty() == false);

    std::uniform_int_distribution<int> distr(0, availableTypesLocal.size()-1);

    // determine cases where special upgrades have to be generated
    std::stack<UpgradeType> mandatoryUpgrades;
    if (isUpgradedToMax(controller, FastBullets) && isUpgradedToMax(controller, PowerBullets))
        mandatoryUpgrades.push(PiercingBullets);
    if (isUpgradedToMax(controller, TankSpeed) && isUpgradedToMax(controller, TankArmor))
        mandatoryUpgrades.push(BulletTank);
    if (isUpgradedToMax(controller, MoreBullets) && isUpgradedToMax(controller, PowerBullets))
        mandatoryUpgrades.push(FourDirectionBullets);
    if (isUpgradedToMax(controller, PowerBullets) && isUpgradedToMax(controller, BonusEffectiveness))
        mandatoryUpgrades.push(Rocket);
    if (isUpgradedToMax(controller, MoreBullets) && isUpgradedToMax(controller, FastBullets))
        mandatoryUpgrades.push(MachineGun);
    if (isUpgradedToMax(controller, FastReload) && isUpgradedToMax(controller, PowerBullets))
        mandatoryUpgrades.push(ReloadOnKill);
    if (isUpgradedToMax(eagleController, RepairWalls) && isUpgradedToMax(eagleController, BaseInvincibility))
        mandatoryUpgrades.push(BaseRestoreOnDamage);

    std::unordered_set<UpgradeType> alreadyGenerated;

    for (int i = 0; i < globalConst::NumOfUpgradesOnLevelup; i++) {
        UpgradeType newType = None;
        int count = 100;
        do {
            int index = distr(Utils::generator);

            UpgradeType t = None;
            if (mandatoryUpgrades.empty()) {
                t = availableTypesLocal[index];
            } else {
                t = mandatoryUpgrades.top();
                mandatoryUpgrades.pop();
            }

            if (alreadyGenerated.contains(t))
                continue;

            newType = t;
        } while (newType == None && --count);

        alreadyGenerated.insert(newType);

        if (count == 0) {
            Logger::instance() << "[ERROR] Could not generate upgrade!\n";
            return;
        }

        int oldLevel = controller->hasLevelOfUpgrade(newType);

        // if is one time bonus
        if (oneTimeBonusTypes.contains(newType)) {
            // and player has effectiveness modifier
            oldLevel = controller->hasLevelOfUpgrade(BonusEffectiveness);
        }

        if (baseUpgradeTypes.contains(newType))
            oldLevel = eagleController->hasLevelOfUpgrade(newType);

        currentRandomUpgrades.push_back(createUpgrade(newType, oldLevel+1));
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
        case FastReload:
            newUpgrade = new FastReloadUpgrade(level);
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
        case FourDirectionBullets:
            newUpgrade = new FourDirectionTurretUpgrade(level);
            break;
        case Rocket:
            newUpgrade = new RocketUpgrade(level);
            break;
        case BulletTank:
            newUpgrade = new BulletTankUpgrade(level);
            break;
        case MachineGun:
            newUpgrade = new MachineGunUpgrade(level);
            break;
        case ReloadOnKill:
            newUpgrade = new ReloadOnKillUpgrade(level);
            break;
        case SacrificeLifeForBase:
            newUpgrade = new SacrificeLifeForBaseUpgrade(level);
            break;
        case PlusLifeOnStart:
            newUpgrade = new TankAdditionalLifePerk();
            break;
        case FavoriteTank:
            newUpgrade = new FavoriteTankPerk();
            break;
        case FavoriteBase:
            newUpgrade = new FavoriteBasePerk();
            break;
        case BaseRestoreOnDamage:
            newUpgrade = new BaseRepairAfterDamageUpgrade();
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

    _iconRect = AssetManager::instance().getAnimationFrame("freezeCollectable", "default", 0).rect;
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

    _percentBasedOnLevel = { 40, 60, 80, 100 };
    for (auto percent : _percentBasedOnLevel)
        _effects.push_back("Every enemy tank\nwill be destroyed\nwith " + std::to_string(percent) + "\% chance");

    _iconRect = AssetManager::instance().getAnimationFrame("grenadeCollectable", "default", 0).rect;
}

void KillEnemiesBonus::onCollect(GameObject *)
{
    assert(_currentLevel < _percentBasedOnLevel.size());
    // kill all enemy tanks (with probability)in a moment

    std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcDoubleCannonArmorTank"});
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
    controller->setTemporaryInvincibility(_timeBasedOnLevel[_currentLevel] * 1000);
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

    for (int i = 0; i < _numberBasedOnLevel[_currentLevel]; i++)
        globalVars::player1Lives++;
}

TankAdditionalLifePerk::TankAdditionalLifePerk()
: TankAdditionalLifeBonus(0)
{
    _currentLevel = 0;
    _category = PlayerUpgrade::Perk;
    _type = PlayerUpgrade::PlusLifeOnStart;
    _price = perkPrices.at(_type);
    _name = "Spare tank";

    _effects.push_back("Start with +1 life");

    _iconRect = AssetManager::instance().getAnimationFrame("plusTankCollectable", "default", 0).rect;
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

    _effects.push_back("turns all your XP to $\nand put it on deposit\n$ can be used in shop later\nall xp and tank upgrades\nwill be reset");


    _iconRect = AssetManager::instance().getAnimationFrame("depositXpCollectable", "default", 0).rect;

}

void DepositXpBonus::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    PlayerController *controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    int xpToSave = controller->xp();
    controller->resetXP();
    controller->updateAppearance();

    PersistentGameData::instance().addToXpDeposit(xpToSave);
}

///////////////


///////////////

FasterBulletUpgrade::FasterBulletUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = FastBullets;
    _name = "Fast delivery";

    _percentBasedOnLevel = { 50, 50, 100, 100 };
    _effects.push_back("Bullet speed +" + std::to_string(_percentBasedOnLevel[0]) + "\%");
    _effects.push_back("Shooting speed +" + std::to_string(_percentBasedOnLevel[1]) + "\%");
    _effects.push_back("Bullet speed +" + std::to_string(_percentBasedOnLevel[2]) + "\%");
    _effects.push_back("Shooting speed +" + std::to_string(_percentBasedOnLevel[3]) + "\%");

    _iconRect = AssetManager::instance().getAnimationFrame("fastBulletCollectable", "default", 0).rect;
}

void FasterBulletUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<Shootable>();
    assert(shootable != nullptr);

    auto controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);

    assert(_currentLevel < _percentBasedOnLevel.size());

    int bulletModifier = 0;
    int timeoutModifier = 0;

    switch (_currentLevel) {
        case 0:
            bulletModifier =  _percentBasedOnLevel[0];
            break;
        case 1:
            bulletModifier =  _percentBasedOnLevel[0];
            timeoutModifier = _percentBasedOnLevel[1];
            break;
        case 2:
            bulletModifier =  _percentBasedOnLevel[2];
            timeoutModifier = _percentBasedOnLevel[1];
            break;
        case 3:
            bulletModifier =  _percentBasedOnLevel[2];
            timeoutModifier = _percentBasedOnLevel[3];
            break;
        default:
            assert(false); // we mustn't be here;
    }

    const int newBulletSpeed = globalConst::DefaultPlayerBulletSpeed + (globalConst::DefaultPlayerBulletSpeed * bulletModifier / 100);
    shootable->setBulletSpeed(newBulletSpeed);

    const int newShootTimeout = globalConst::PlayerShootTimeoutMs - (globalConst::PlayerShootTimeoutMs * timeoutModifier / 100);
    shootable->setShootTimeoutMs(newShootTimeout);
}

///////////////
FastReloadUpgrade::FastReloadUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = FastReload;
    _name = "Fast Reload";

    _percentBasedOnLevel = { 10, 20, 30, 40 };
    for (auto number : _percentBasedOnLevel)
        _effects.push_back("+" + std::to_string(number) + "\% ammo reload speed");

    _iconRect = AssetManager::instance().getAnimationFrame("fastReloadCollectable", "default", 0).rect;
}

void FastReloadUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<Shootable>();
    assert(shootable != nullptr);
    assert(_currentLevel < _percentBasedOnLevel.size());

    int newReloadTimeout = globalConst::PlayerDefaultReloadTimeoutMs - globalConst::PlayerDefaultReloadTimeoutMs * _percentBasedOnLevel[_currentLevel]/100;
    shootable->setReloadTimeoutMs(newReloadTimeout);
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
        _effects.push_back("+" + std::to_string(number) + " bullets in ammutition");

    _iconRect = AssetManager::instance().getAnimationFrame("ammoPlusCollectable", "default", 0).rect;
}

void MoreBulletsUpgrade::onCollect(GameObject *collector)
{
    assert(collector != nullptr);
    auto shootable = collector->getComponent<Shootable>();
    assert(shootable != nullptr);
    assert(_currentLevel < _numberBasedOnLevel.size());

    int newMaxBullets = globalConst::PlayerDefaultMaxBullets + _numberBasedOnLevel[_currentLevel];
    shootable->setMaxBullets(newMaxBullets);
}


///////////////

ArmorUpgrade::ArmorUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::TankUpgrade;
    _type = TankArmor;
    _name = "Coat of steel";

    _numberBasedOnLevel = { 1, 2, 3, 4 };
    _debuffBasedOnLevel = { 5, 10, 15, 20 };
    for (int i=0; i<_numberBasedOnLevel.size(); i++) {
        const int buff = _numberBasedOnLevel[i];
        const int debuff = _debuffBasedOnLevel[i];
        const std::string caption = std::format("Tank protection +{}\nMove speed -{}\%", buff, debuff);
        _effects.push_back(caption);
    }

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

    auto controller = collector->getComponent<PlayerController>();
    controller->addToCalculatedSpeedDebuff(_debuffBasedOnLevel[_currentLevel]);
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
    controller->setMoveSpeed(newTankSpeed);
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
        _effects.push_back("Bullet damage +" + std::to_string(number) + "\nReload delay +25\%");

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

    auto controller = collector->getComponent<PlayerController>();
    assert(controller != nullptr);
    controller->addToCalculatedReloadDebuff(25);
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


FourDirectionTurretUpgrade::FourDirectionTurretUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type  = FourDirectionBullets;
    _name = "Cross fire";

    _effects.push_back("Replaces [Power Bullets] and\n[Productive turret] upgrades\nNew turret shoots in all\n4 directions at once");

    _iconRect = AssetManager::instance().getAnimationFrame("fourBulletsCollectable", "default", 0).rect;
}

void FourDirectionTurretUpgrade::onCollect(GameObject *target)
{
    // nothing is needed except the fact of presense of this effect
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Player));
    auto controller = target->getComponent<PlayerController>();
    controller->setFourDirectionTurret();
}

/////////////

RocketUpgrade::RocketUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type  = Rocket;
    _name = "Rocket launcher";

    _effects.push_back("Replaces [Power Bullets] and\n[Effective Electronics] upgrades\nNew turret shoots rockets,\nwhich explode and affect damage");

    _iconRect = AssetManager::instance().getAnimationFrame("redRocketCollectable", "default", 0).rect;
}

void RocketUpgrade::onCollect(GameObject *target)
{
    // nothing is needed except the fact of presense of this effect
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Player));
    auto controller = target->getComponent<PlayerController>();
    controller->setRocketLauncher();
}
/////////////

MachineGunUpgrade::MachineGunUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type  = MachineGun;
    _name = "Machine gun";

    _effects.push_back("Replaces [Productive turret] and\n[Fast delivery] upgrades\nNew turret shoots limitless,\nfast bullets at rapid speed");

    _iconRect = AssetManager::instance().getAnimationFrame("machineGunCollectable", "default", 0).rect;
}

void MachineGunUpgrade::onCollect(GameObject *target)
{
    // nothing is needed except the fact of presense of this effect
    assert(target != nullptr);
    assert(target->isFlagSet(GameObject::Player));

    auto shootable = target->getComponent<Shootable>();
    assert(shootable != nullptr);

    // bullet speed
    const int newBulletSpeed = globalConst::DefaultPlayerBulletSpeed + (globalConst::DefaultPlayerBulletSpeed * 80 / 100);
    shootable->setBulletSpeed(newBulletSpeed);

    // shoot speed
    shootable->setShootTimeoutMs(globalConst::PlayerShootTimeoutMs / 2);

    // set max ammo
    shootable->setMaxBullets(10);

    // set damage
    //shootable->setDamage(std::max(shootable->damage()/2, 1));

    // set instant reload
    shootable->setInstantReload(true);

}

///

ReloadOnKillUpgrade::ReloadOnKillUpgrade(int level)
: PlayerUpgrade(level)
{
    _synergic = true;
    _category = PlayerUpgrade::TankUpgrade;
    _type  = ReloadOnKill;
    _name = "Harvester of sorrow";

    _effects.push_back("Replaces [Fast reload] and\n[power bullets] upgrades\nAmmo will fully reload\nafter every succesful kill");

    _iconRect = AssetManager::instance().getAnimationFrame("skullCollectable", "default", 0).rect;
}

void ReloadOnKillUpgrade::onCollect(GameObject *target)
{
    // nothing is needed except the fact of presense of this effect
}

//////////

XpModifierUpgrade::XpModifierUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _type = XpIncreaser;
    _price = perkPrices.at(_type);
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
    assert(controller != nullptr);
    controller->setXpModifier(_numberBasedOnLevel[_currentLevel]);
}

//////////

KillAllOnTankDeathUpgrade::KillAllOnTankDeathUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _type = KillAllOnDeath;
    _price = perkPrices.at(_type);
    _name = "Atomic Core";

    _effects.push_back("When your tank dies,\nall the enemies will perish");

    _iconRect = AssetManager::instance().getAnimationFrame("redTankCollectable", "default", 0).rect;
}

void KillAllOnTankDeathUpgrade::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}

/// @brief ////////////////
/// @param level /

SacrificeLifeForBaseUpgrade::SacrificeLifeForBaseUpgrade(int level)
: PlayerUpgrade(level)
{
    _category = PlayerUpgrade::Perk;
    _type = SacrificeLifeForBase;
    _price = perkPrices.at(_type);
    _name = "Sacrifice";

    _effects.push_back("Sacrifice 1 life once to revive\nthe base and repair its walls");

    _iconRect = AssetManager::instance().getAnimationFrame("redEagleCollectable", "default", 0).rect;
}

void SacrificeLifeForBaseUpgrade::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}


/////////

FavoriteTankPerk::FavoriteTankPerk()
: PlayerUpgrade(0)
{
    _category = PlayerUpgrade::Perk;
    _type = FavoriteTank;
    _price = perkPrices.at(_type);
    _name = "Selfish";

    _effects.push_back("More chance to obtain\ntank upgrades");

    _iconRect = AssetManager::instance().getAnimationFrame("tankCrownCollectable", "default", 0).rect;
}

void FavoriteTankPerk::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}

/////////

FavoriteBasePerk::FavoriteBasePerk()
: PlayerUpgrade(0)
{
    _category = PlayerUpgrade::Perk;
    _type = FavoriteBase;
    _price = perkPrices.at(_type);
    _name = "Duty above all!";

    _effects.push_back("More chance to obtain\nbase upgrades");

    _iconRect = AssetManager::instance().getAnimationFrame("eagleCrownCollectable", "default", 0).rect;
}

void FavoriteBasePerk::onCollect(GameObject *)
{
     // nothing is needed except the fact of presense of this effect
}