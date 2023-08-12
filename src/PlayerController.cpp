#include "Controller.h"
#include "GameObject.h"
#include "Damageable.h"
#include "Shootable.h"
#include "SoundPlayer.h"
#include "Utils.h"


PlayerController::PlayerController(GameObject *obj)
: Controller(obj)
{

}


void PlayerController::setPressedFlag(KeysPressed flag, bool state)
{
    if (state)
        _pressedStates = _pressedStates | flag;
    else
        _pressedStates &= ~flag;
}

bool PlayerController::wasPressed(KeysPressed flag)
{
    return (_pressedStates & flag) != 0;
}

void PlayerController::update()
{
    assert(_gameObject != nullptr);
    if (_invincible) {
        if (_invincibilityTimer.getElapsedTime() < sf::seconds(_invincibilityTimeout))
            _gameObject->visualEffect->copyParentPosition(_gameObject);
        else {
            _invincible = false;
            delete _gameObject->visualEffect;
            _gameObject->visualEffect = nullptr;
            Damageable *d = _gameObject->getComponent<Damageable>();
            d->makeInvincible(false);
        }
    }
    bool action = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        if (_gameObject->shoot())
            SoundPlayer::instance().playShootSound();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        if (!wasPressed(LeftPressed))
            _pressedKeys[sf::Keyboard::Left] = _clock.getElapsedTime();
        setPressedFlag(LeftPressed, true);
    } else {
        setPressedFlag(LeftPressed, false);
        _pressedKeys.erase(sf::Keyboard::Left);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        if (!wasPressed(UpPressed))
            _pressedKeys[sf::Keyboard::Up] = _clock.getElapsedTime();
        setPressedFlag(UpPressed, true);
    } else {
        setPressedFlag(UpPressed, false);
        _pressedKeys.erase(sf::Keyboard::Up);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        if (!wasPressed(RightPressed))
            _pressedKeys[sf::Keyboard::Right] = _clock.getElapsedTime();
        setPressedFlag(RightPressed, true);
    } else {
        setPressedFlag(RightPressed, false);
        _pressedKeys.erase(sf::Keyboard::Right);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        if (!wasPressed(DownPressed))
            _pressedKeys[sf::Keyboard::Down] = _clock.getElapsedTime();
        setPressedFlag(DownPressed, true);
    } else {
        setPressedFlag(DownPressed, false);
        _pressedKeys.erase(sf::Keyboard::Down);
    }

    auto recentKey = sf::Keyboard::Unknown;
    sf::Time latestTime = sf::milliseconds(0);

    for (auto pk : _pressedKeys) {
        if (pk.second > latestTime) {
            recentKey = pk.first;
            latestTime = pk.second;
        }
    }

    int speed = ((int)(moveSpeed * Utils::lastFrameTime.asSeconds()) >> 1) << 1;

    switch (recentKey) {
        case sf::Keyboard::Left:
            _gameObject->move(-speed, 0);
            _gameObject->setCurrentDirection(globalTypes::Left);
            _gameObject->setCurrentAnimation("left");
            _isMoving = true;
            break;
        case sf::Keyboard::Up:
            _gameObject->move(0, -speed);
            _gameObject->setCurrentDirection(globalTypes::Up);
            _gameObject->setCurrentAnimation("up");
            _isMoving = true;
            break;
        case sf::Keyboard::Right:
            _gameObject->move(speed, 0);
            _gameObject->setCurrentDirection(globalTypes::Right);
            _gameObject->setCurrentAnimation("right");
            _isMoving = true;
            break;
        case sf::Keyboard::Down:
            _gameObject->move(0, speed);
            _gameObject->setCurrentDirection(globalTypes::Down);
            _gameObject->setCurrentAnimation("down");
            _isMoving = true;
        default:
            _isMoving = false;
    }

    if (_isMoving) {
        _lastActionTime = _clock.getElapsedTime();
        SoundPlayer::instance().playTankMoveSound();
        _gameObject->restartAnimation();
    } else {
        SoundPlayer::instance().playTankStandSound();
        _gameObject->stopAnimation();
    }
}

void PlayerController::increasePowerLevel(bool inc)
{
    if (inc  && _powerLevel<3) _powerLevel++;
    if (!inc && _powerLevel>0) _powerLevel--;

    updatePowerLevel();
}

void PlayerController::updatePowerLevel()
{
    SpriteRenderer *renderer = _gameObject->getComponent<SpriteRenderer>();
    assert(renderer != nullptr);

    PlayerShootable *shootable = _gameObject->getComponent<PlayerShootable>();
    assert(shootable != nullptr);

    using namespace globalConst;

    switch (_powerLevel) {
        case 0:
            shootable->resetLevel();
            shootable->setDamage(DefaultDamage);
            shootable->setBulletSpeed(DefaultBulletSpeed);
            renderer->setSpriteSheetOffset(0, 0);
        case 1:
            shootable->resetLevel();
            shootable->setDamage(DefaultDamage);
            shootable->setBulletSpeed(DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 2:
            shootable->resetLevel();
            shootable->increaseLevel();
            shootable->setDamage(DefaultDamage);
            shootable->setBulletSpeed(DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 3:
            shootable->resetLevel();
            shootable->increaseLevel();
            shootable->setDamage(DoubleDamage);
            shootable->setBulletSpeed(DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 48);
            break;
    }

    globalVars::player1PowerLevel = _powerLevel;
}

void PlayerController::setTemporaryInvincibility(int sec)
{
    _invincible = true;
    _invincibilityTimeout = sec;
    _invincibilityTimer.restart();
    Damageable *dmg = _gameObject->getComponent<Damageable>();
    dmg->makeInvincible(true);

    if (_gameObject->visualEffect == nullptr) {
        GameObject *cloud = new GameObject(_gameObject, "cloud");
        cloud->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        cloud->setRenderer(new LoopAnimationSpriteRenderer(cloud, "cloud"));
        _gameObject->visualEffect = cloud;
    }

}