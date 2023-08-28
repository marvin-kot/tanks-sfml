#pragma once

#include <string>
#include <map>
#include <random>
#include <utility>

#include <SFML/Graphics/Rect.hpp>

#include <unordered_set>


class GameObject;
class PlayerUpgrade
{
public:
    enum UpgradeType {
        None,
        // tank upgrades
        FastBullets,
        MoreBullets,
        TankSpeed,
        PowerBullets,
        TankArmor,
        XpAttractor,
        BonusEffectiveness,
        // tank super upgrades
        FourDirectionBullets,
        PiercingBullets,
        Rocket,
        BulletTank,
        // base upgrades
        BaseArmor,
        RepairWalls,
        BaseRestoreHP,
        ReflectBullets,
        PhoenixBase,
        BaseInvincibility,
        BaseRevengeOnDamage,
        // one time bonuses
        FreezeEnemies,
        InstantKillEnemies,
        TempConcreteBaseWalls,
        TempTankInvincibility,
        AdditionalLife,
        RandomWalls,
        InstantKillBaseArea,
        CollectAllFreeXP,
        RebuildEagleWalls,
        DepositXP,
        // perks (tank upgrades bouht before run)
        PlusLifeOnStart,
        XpIncreaser,
        KillAllOnDeath,
        SacrificeLifeForBase,
    };

    enum UpgradeCategory
    {
        TankUpgrade,
        BaseUpgrade,
        OneTimeBonus,
        Perk
    };

    // TODO: incapsulate
    static std::vector<UpgradeType> availableTypes;
    static void removeFromAvailable(UpgradeType);
protected:
    GameObject *_parent;

    UpgradeType _type;
    UpgradeCategory _category;
    int _currentLevel;
    bool _synergic = false;
    std::map<UpgradeType, int> _dependencies;

    std::string _name;
    std::vector<std::string> _effects;

    sf::IntRect _iconRect;

    bool _applied = false;
    int _price = 0; // for perks only
public:
    PlayerUpgrade(int level);

    virtual void onCollect(GameObject *collector) = 0;

    std::string name() const;
    UpgradeType type() const { return _type; }
    UpgradeCategory category() const { return _category; }
    int currentLevel() const { return _currentLevel; }
    int maxLevel() const { return _effects.size(); }
    void increaseLevel() { _currentLevel++; }
    std::string currentEffectDescription() const;
    sf::IntRect iconRect() const { return _iconRect; }
    std::pair<UpgradeType, int> dependency() const;
    bool firstApplication();
    void reset();

    int price() const { return _price; }

// STATIC methods
public:
    static PlayerUpgrade *createUpgrade(UpgradeType type, int level);
    static std::vector<PlayerUpgrade *> currentRandomUpgrades;
    static std::vector<PlayerUpgrade *> availablePerkObjects;
    static std::unordered_set<UpgradeType> playerOwnedPerks;
    static void generateRandomUpgradesForPlayer(GameObject *playerObject);
    static void generatePerks();
    static void deletePerks();
};


class BonusEffectivenessIncreaser : public PlayerUpgrade
{
public:
    BonusEffectivenessIncreaser(int level);
    void onCollect(GameObject *collector) override;
};


class FreezeEnemiesBonus : public PlayerUpgrade
{
    std::vector<int> _timeoutBasedOnLevel;

public:
    FreezeEnemiesBonus(int level);
    void onCollect(GameObject *collector) override;
};

class KillEnemiesBonus : public PlayerUpgrade
{
    std::vector<int> _percentBasedOnLevel;
    std::uniform_int_distribution<int> distribution;

public:
    KillEnemiesBonus(int level);
    void onCollect(GameObject *collector) override;
};

class TankInvincibilityBonus : public PlayerUpgrade
{
    std::vector<int> _timeBasedOnLevel;

public:
    TankInvincibilityBonus(int level);
    void onCollect(GameObject *collector) override;
};

class TankAdditionalLifeBonus : public PlayerUpgrade
{
protected:
    std::vector<int> _numberBasedOnLevel;
public:
    TankAdditionalLifeBonus();
    TankAdditionalLifeBonus(int level);
    void onCollect(GameObject *collector) override;
};

class TankAdditionalLifePerk : public TankAdditionalLifeBonus
{
public:
    TankAdditionalLifePerk();
};

class RebuildEagleWallsBonus : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    RebuildEagleWallsBonus(int level);
    void onCollect(GameObject *collector) override;
};

class DepositXpBonus : public PlayerUpgrade
{
public:
    DepositXpBonus(int level);
    void onCollect(GameObject *collector) override;
};


class RebuildEagleWallsOnLevelup : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
    int levelupCounter = 0;

public:
    RebuildEagleWallsOnLevelup(int level);
    void onCollect(GameObject *collector) override;
};

class BaseArmorUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    BaseArmorUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class EagleInvincibilityAfterDamage : public PlayerUpgrade
{
    std::vector<int> _timeBasedOnLevel;
public:
    EagleInvincibilityAfterDamage(int level);
    void onCollect(GameObject *collector) override;
};

class FasterBulletUpgrade : public PlayerUpgrade
{
    std::vector<int> _percentBasedOnLevel;
public:
    FasterBulletUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class MoreBulletsUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    MoreBulletsUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class ArmorUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    ArmorUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class FasterTankUpgrade : public PlayerUpgrade
{
    std::vector<int> _percentBasedOnLevel;
public:
    FasterTankUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class PowerBulletUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    PowerBulletUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class XpModifierUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    XpModifierUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class PiercingBulletsUpgrade : public PlayerUpgrade
{
public:
    PiercingBulletsUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class BulletTankUpgrade : public PlayerUpgrade
{
public:
    BulletTankUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class FourDirectionTurretUpgrade : public PlayerUpgrade
{
public:
    FourDirectionTurretUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class KillAllOnTankDeathUpgrade : public PlayerUpgrade
{
public:
    KillAllOnTankDeathUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

class SacrificeLifeForBaseUpgrade : public PlayerUpgrade
{
public:
    SacrificeLifeForBaseUpgrade(int level);
    void onCollect(GameObject *collector) override;
};

////// PERKS
/*  PlayerUpgrade::KillAllOnDeath,
    PlayerUpgrade::SacrificeLifeForBase,
    PlayerUpgrade::PlusLifeOnStart,
    PlayerUpgrade::GreedForXP*/