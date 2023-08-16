#pragma once

#include "Controller.h"
#include "PlayerUpgrade.h"

#include <SFML/Window/Keyboard.hpp>

#include <map>


class PlayerController : public Controller
{
    sf::Keyboard::Key _mostRecentKey;
    enum KeysPressed {
        NothingPressed  = 0x0,
        SpacePressed    = 0x1,
        LeftPressed     = 0x2,
        UpPressed       = 0x4,
        RightPressed    = 0x8,
        DownPressed     = 0x10
    };

    int _pressedStates = 0;

    void setPressedFlag(KeysPressed flag, bool state);
    bool wasPressed(KeysPressed flag);

    std::map<sf::Keyboard::Key, sf::Time> _pressedKeys;

    std::map<PlayerUpgrade::UpgradeType, PlayerUpgrade *> _collectedUpgrades;

    int _powerLevel = 0;
    int _xp;
    int _level = 0;

    bool _invincible;
    sf::Clock _invincibilityTimer;
    int _invincibilityTimeout;

    void trySqueeze();

public:
    PlayerController(GameObject *obj);
    ~PlayerController();
    void update() override;
    int powerLevel() const { return _powerLevel; }
    void increasePowerLevel(bool);
    void updatePowerLevel();
    void applyUpgrades();
    void updateAppearance();
    void updateMoveSpeed(int speed) { _moveSpeed = speed; }
    int numberOfUpgrades() const;
    void setTemporaryInvincibility(int sec);
    void addXP(int val);
    void resetXP();
    void levelUp();

    void chooseUpgrade(int index);
    int hasLevelOfUpgrade(PlayerUpgrade::UpgradeType) const;
    PlayerUpgrade *getUpgrade(int) const;
};

struct PlayerSignal
{
    int pid;
    int direction; // 1-4
    bool shoot;
    int moveX, moveY;
};

struct PlayerState
{
    int pid;
    int power;
    int lives;
    int direction;
    int coordX, coordY;
};

