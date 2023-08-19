#pragma once

#include "Controller.h"
#include "GlobalTypes.h"
#include "PlayerUpgrade.h"

#include <sftools/Chronometer.hpp>


#include <map>
#include <stack>

class EagleController : public Controller
{
    enum EagleStates {
        Starting,
        BuildingWalls,
        Waiting,
        Reswawning
    };

    EagleStates _state;
    std::vector<GameObject *> _surroundingWalls;

    std::map<PlayerUpgrade::UpgradeType, PlayerUpgrade *> _collectedUpgrades;

    globalTypes::EagleWallDirection _currentBuildDirection;

    int _fastRebuildTimeoutMs = 150;
    int _slowRebuildTimeoutMs = 3000;
    bool _isSlowRepairMode = false;

    std::stack<int> _rebuildTimeouts;

    bool _invincibilityAfterDamageHit = false;
    int _invincibilityAfterDamageTimeout = 0;
    bool _invincible;
    sftools::Chronometer _invincibilityTimer;
public:
    EagleController(GameObject *parent);
    ~EagleController();

    void update() override;
    void updateAppearance();

    void upgrade(PlayerUpgrade *);
    void applyUpgrades();

    int hasLevelOfUpgrade(PlayerUpgrade::UpgradeType) const;
    PlayerUpgrade *getUpgrade(int) const;
    int numberOfUpgrades() const;

    void fastRepairWalls(int);
    void setSlowRepairMode(int);

    void setTempInvincibilityAfterDamage(int timeout);
    void onDamaged() override;
};