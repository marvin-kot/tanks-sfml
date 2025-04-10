#pragma once

#include <SFML/System/Time.hpp>

#include <vector>

class GameObject;

class Collectable
{
    GameObject *_gameObject;

public:
    Collectable(GameObject *parent);
    virtual bool onCollected(GameObject *collector); // Changed return type to bool
};

struct GrenadeCollectable : public Collectable
{
    GrenadeCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

struct TimerCollectable : public Collectable
{
    TimerCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

struct HelmetCollectable : public Collectable
{
    HelmetCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

struct TankCollectable : public Collectable
{
    TankCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

struct XpCollectable : public Collectable
{

    XpCollectable(GameObject *parent, int value);
    bool onCollected(GameObject *collector) override; // Changed return type to bool

    int value() const { return _value; }
    void setValue(int val) { _value = val; }

private:
    int _value;
};

struct AmmoCollectable : public Collectable
{
    const int _amount = 3;
    AmmoCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

struct RepairCollectable : public Collectable
{
    RepairCollectable(GameObject *parent);
    bool onCollected(GameObject *collector) override; // Changed return type to bool
};

class PlayerUpgrade;

struct SkullCollectable : public Collectable
{
    sf::Time dropTime;
    int xp = 0;
    int level = 0;

    std::vector<PlayerUpgrade *> playerUpgrades;

    SkullCollectable(GameObject *parent);
    ~SkullCollectable();
    bool onCollected(GameObject *collector) override; // Changed return type to bool

private:
    bool _gotCollected = false;
};