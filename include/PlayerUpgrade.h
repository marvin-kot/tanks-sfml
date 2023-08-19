#pragma once

#include <string>
#include <map>
#include <random>
#include <utility>

#include <SFML/Graphics/Rect.hpp>

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
        Rocket,
        // base upgrades
        BaseArmor,
        RepairWalls,
        BaseRestoreHP,
        ReflectBullets,
        PhoenixBase,
        BaseInvincibility,
        // one time bonuses
        FreezeEnemies,
        InstantKillEnemies,
        TempConcreteBaseWalls,
        TempTankInvincibility,
        AdditionalLife,
        RandomWalls,
        InstantKillBaseArea,
        CollectAllFreeXP,
        RebuildEagleWalls
    };

    enum UpgradeCategory
    {
        TankUpgrade,
        BaseUpgrade,
        OneTimeBonus
    };
protected:
    GameObject *_parent;

    UpgradeType _type;
    UpgradeCategory _category;
    int _currentLevel;
    std::map<UpgradeType, int> _dependencies;

    std::string _name;
    std::vector<std::string> _effects;

    sf::IntRect _iconRect;
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

// STATIC methods
public:
    static PlayerUpgrade *createUpgrade(UpgradeType type, int level);
    static std::vector<PlayerUpgrade *> currentThreeRandomUpgrades;
    static void generateThreeRandomUpgradesForPlayer(GameObject *playerObject);
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
    std::vector<int> _numberBasedOnLevel;
public:
    TankAdditionalLifeBonus(int level);
    void onCollect(GameObject *collector) override;
};

class RebuildEagleWallsBonus : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    RebuildEagleWallsBonus(int level);
    void onCollect(GameObject *collector) override;
};

class EagleInvincibilityAfterDamage : public PlayerUpgrade
{
    std::vector<int> _timeBasedOnLevel;
public:
    EagleInvincibilityAfterDamage(int level);
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

class BaseArmorUpgrade : public PlayerUpgrade
{
    std::vector<int> _numberBasedOnLevel;
public:
    BaseArmorUpgrade(int level);
    void onCollect(GameObject *collector) override;
};