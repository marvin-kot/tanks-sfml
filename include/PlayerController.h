#pragma once

#include "Controller.h"
#include "NetGameTypes.h"
#include "PlayerUpgrade.h"

#include <SFML/Window/Keyboard.hpp>

#include <map>

#define SINGLE_APP


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

    int _xp;
    int _level = 0;

    bool _invincible;
    sf::Clock _invincibilityTimer;
    int _invincibilityTimeout;

    sftools::Chronometer _moveStartClock;

    int trySqueeze();
    bool _prevMoved = false;

    net::PlayerInput _recentPlayerInput;

    int _xpModifier = 0;

    bool _4dirSet = false;

public:
    PlayerController(GameObject *obj);
    ~PlayerController();
    void update() override;
    void applyUpgrades();
    void updateAppearance();
    void updateMoveSpeed(int speed) { _moveSpeed = speed; }
    int numberOfUpgrades() const;
    void setTemporaryInvincibility(int msec);
    void addXP(int val);
    int xp() const { return _xp; }
    void resetXP();
    void levelUp();

    void chooseUpgrade(int index);
    int hasLevelOfUpgrade(PlayerUpgrade::UpgradeType) const;
    PlayerUpgrade *getUpgrade(int) const;
    void removeUpgrade(PlayerUpgrade::UpgradeType);
    void onDamaged() override;
    void setXpModifier(int mod) { _xpModifier = mod; }
    int xpModifier() const { return _xpModifier; }
    void setPlayerInput(const net::PlayerInput& input) { _recentPlayerInput = input; }
    void resetMoveStartTimer();

    void setFourDirectionTurret();
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

