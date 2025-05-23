#pragma once

#include <SFML/System/Clock.hpp>

class GameObject;

class Damageable
{
private:
    GameObject *_gameObject;
    int _def;
    int _maxDefence;
    bool _invincible;

    sf::Clock _clock;
public:
    Damageable(GameObject *parent, int def);
    void takeDamage(int dmg);
    bool isDestroyed() const;

    void setDefence(int def);
    int defence() const;

    int maxDefence() const;

    void restoreDefence();

    void makeInvincible(bool);
    bool isInvincible() const;

    void setInvincibilityOnHit(int);
};
