#pragma once

#include "PlayerUpgrade.h"

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


class BaseRepairAfterDamageUpgrade : public PlayerUpgrade
{
    std::vector<int> _timeBasedOnLevel;
public:
    BaseRepairAfterDamageUpgrade();
    void onCollect(GameObject *collector) override;
};