#pragma once

#include "Controller.h"
#include "NetGameTypes.h"
#include "PlayerUpgrade.h"

#include <SFML/Window/Keyboard.hpp>

#include <map>

class SkullCollectable;

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

    long _xp;
    int _level = 0;

    bool _invincible;
    sf::Clock _invincibilityTimer;
    int _invincibilityTimeout;

    sftools::Chronometer _moveStartClock;

    int trySqueeze();
    bool _prevMoved = false;

    bool _upgradeSpeed = false;
    bool _upgradePower = false;

    net::PlayerInput _recentPlayerInput;

    int _xpModifier = 0;

    bool _4dirSet = false;
    bool _rocketSet = false;

    int _speedDebuff = 0;
    int _shootReloadDebuff = 0;

    bool _blink = false;

    int _afterShootDirectionDelayCounter = 0;

    void blinkIfParalyzed();
    sf::Time prevFrameTime;
public:
    PlayerController(GameObject *obj);
    ~PlayerController();
    void update() override;
    GameObject *onDestroyed() override;
    void applyPerks();
    void removePerks();
    void applyUpgrades();
    void updateAppearance();
    void setMoveSpeed(int speed);
    int moveSpeed() const { return _moveSpeed; }
    int numberOfUpgrades() const;
    void setTemporaryInvincibility(int msec);
    void addXP(int val);
    int xp() const { return _xp; }
    int level() const { return _level; }
    void resetXP();
    void levelUp();

    bool previouslyMoved() const override{ return _prevMoved; }

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
    void setRocketLauncher();

    void setSpeedDebuff(int d) { _speedDebuff = d; }
    void setReloadDebuff(int d);

    void resetCalculatedSpeedDebuff() {_speedDebuff = 0; }
    void addToCalculatedSpeedDebuff(int d) { _speedDebuff += d; }
    void applyCalculatedSpeedDebuff();

    void resetCalculatedReloadDebuff() {_shootReloadDebuff = 0; }
    void addToCalculatedReloadDebuff(int d) { _shootReloadDebuff += d; }
    void subtractFromCalculatedReloadDebuff(int d) { _shootReloadDebuff -= d;}
    void applyCalculatedReloadDebuff();

    void onKillEnemy(GameObject *enemy);

    void restoreProtection();

    void restoreLevelAndUpgrades(SkullCollectable *);

    void dropSkull();

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

