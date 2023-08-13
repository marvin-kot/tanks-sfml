#pragma once

#include "Controller.h"

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

    int _powerLevel = 0;

    bool _invincible;
    sf::Clock _invincibilityTimer;
    int _invincibilityTimeout;

public:
    PlayerController(GameObject *obj);
    void update() override;
    int powerLevel() const { return _powerLevel; }
    void increasePowerLevel(bool);
    void updatePowerLevel();
    void setTemporaryInvincibility(int sec);
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

