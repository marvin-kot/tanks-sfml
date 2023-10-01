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

    int _invincibilityAfterDamageTimeout;
    bool _invincible;
    sftools::Chronometer _invincibilityTimer;
    void removeUpgrade(PlayerUpgrade::UpgradeType t);

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
    GameObject *onDestroyed() override;
};