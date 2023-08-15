#pragma once

class GameObject;

class Damageable
{
private:
    GameObject *_gameObject;
    int _hp;
    int _def;
    bool _invincible;

public:
    Damageable(GameObject *parent, int hp);
    void takeDamage(int dmg);
    bool isDestroyed() const;

    void setDefence(int def);
    int defence() const;

    void makeInvincible(bool);
    bool isInvincible() const;
};
