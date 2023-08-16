#pragma once

#include "Controller.h"
#include "PlayerUpgrade.h"


#include <map>

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
};