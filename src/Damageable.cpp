#include "Damageable.h"

#include "GameObject.h"

Damageable::Damageable(GameObject *parent, int hp)
:_gameObject(parent), _hp(hp)
{}

void Damageable::takeDamage(int dmg)
{
    if (_invincible) return;

    if (_def>0) {
        int realDmg = dmg - _def;
        if (realDmg < 0) realDmg = 0;
        _hp -= realDmg;
        int realDef = _def - dmg;
        if (realDef<0) realDef = 0;
        _def = realDef;
    }
    _hp -= dmg;
}

bool Damageable::isDestroyed() const
{
    return _hp < 1;
}

void Damageable::setDefence(int def)
{
    _def = def;
}

void Damageable::makeIncincible(bool val)
{
    _invincible = val;
}

bool Damageable::isInvincible() const
{
    return _invincible;
}