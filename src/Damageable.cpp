#include "Controller.h"
#include "Damageable.h"
#include "GameObject.h"

Damageable::Damageable(GameObject *parent, int def)
:_gameObject(parent), _def(def), _invincible(false)
{}

void Damageable::takeDamage(int dmg)
{
    if (_invincible) return;

    if (_clock.getElapsedTime() < sf::milliseconds(globalConst::MinDamageTimeoutMs))
        return;
    _clock.restart();

    _def = _def - dmg;

    auto controller = _gameObject->getComponent<Controller>();

    if (controller)
        controller->onDamaged();
}

bool Damageable::isDestroyed() const
{
    return _def < 0;
}

void Damageable::setDefence(int def)
{
    _maxDefence = _def = def;
}

int Damageable::defence() const
{
    return _def;
}

int Damageable::maxDefence() const
{
    return _maxDefence;
}

void Damageable::restoreDefence()
{
    _def = _maxDefence;
}

void Damageable::makeInvincible(bool val)
{
    _invincible = val;
}

bool Damageable::isInvincible() const
{
    return _invincible;
}