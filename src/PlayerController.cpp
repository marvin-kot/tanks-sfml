#include "PlayerController.h"

#include "Damageable.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Shootable.h"
#include "SoundPlayer.h"
#include "Utils.h"

PlayerController::PlayerController(GameObject *obj)
: Controller(obj, globalConst::DefaultPlayerSpeed)
{
    _clock.reset(true);
    resetXP();
}

PlayerController::~PlayerController()
{
    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
        delete obj;
    }
    resetXP();
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
    checkForGamePause();
    if (_pause) return;

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


    int speed = ((int)(_moveSpeed * Utils::lastFrameTime.asSeconds()) >> 1) << 1;
    globalTypes::Direction direction = _gameObject->direction();
    switch (recentKey) {
        case sf::Keyboard::Left:
            direction = globalTypes::Left;
            _isMoving = true;
            break;
        case sf::Keyboard::Up:
            direction = globalTypes::Up;
            _isMoving = true;
            break;
        case sf::Keyboard::Right:
            direction = globalTypes::Right;
            _isMoving = true;
            break;
        case sf::Keyboard::Down:
            direction = globalTypes::Down;
            _isMoving = true;
            break;
        default:
            _gameObject->moving = false;
            _isMoving = false;
            break;
    }

    if (_isMoving) {
        prepareMoveInDirection(direction);
        if (_gameObject->move(_currMoveX, _currMoveY) == 0) {
            // try same direction but +1/-1 pixes aside
            trySqueeze();
        }
        _lastActionTime = _clock.getElapsedTime();
        SoundPlayer::instance().playTankMoveSound();
        _gameObject->restartAnimation();
    } else {
        // check if on ice
        if (_gameObject->isOnIce()) {
            SoundPlayer::instance().playIceSkidSound();
            if (_gameObject->move(_currMoveX, _currMoveY) == 0) {
                SoundPlayer::instance().playTankStandSound();
                _gameObject->stopAnimation();
            }
        } else {
            SoundPlayer::instance().playTankStandSound();
            _gameObject->stopAnimation();
        }
    }
}

void PlayerController::trySqueeze()
{
    if (_currMoveX == 0) {
        if (_gameObject->move(2, _currMoveY) == 0)
            _gameObject->move(-2, _currMoveY);
    } else if (_currMoveY == 0) {
        if (_gameObject->move(_currMoveX, 2) == 0)
            _gameObject->move(_currMoveX, -2);
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

void PlayerController::updateAppearance()
{
    SpriteRenderer *renderer = _gameObject->getComponent<SpriteRenderer>();
    assert(renderer != nullptr);
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);

    switch (damageable->defence()) {
        case 0:
            renderer->setSpriteSheetOffset(0, 0);
            break;
        case 1:
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 2:
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 3:
            renderer->setSpriteSheetOffset(0, 48);
            break;
        case 4:
            renderer->setSpriteSheetOffset(0, 48);
            break;
    }

    renderer->showAnimationFrame(0);
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

std::vector<int> xpNeededForLevelUp = {
    400, // 400
    1000, // 600
    2000, // 1000
    3500, // 1500
    5500,  // 2000
    8000, // 2500
    11000, // 3000
    16000, // 5000
    24000 // 8000
 };

void PlayerController::addXP(int val)
{
    _xp += val;
    Logger::instance() << "collect " << val << "xp\n";
    globalVars::player1XP += val;
    if (_xp >= xpNeededForLevelUp[_level]) {
        levelUp();
    }
}

void PlayerController::resetXP()
{
    _xp = 0;
    _level = 0;

    globalVars::player1XP = 0;
    globalVars::player1Level = 1;

    if (ObjectsPool::eagleObject) {
        ObjectsPool::eagleObject->getComponent<Damageable>()->setDefence(0);
        ObjectsPool::eagleObject->getComponent<EagleController>()->updateAppearance();
    }
}

void PlayerController::levelUp()
{
    // if we already lost, no sense to level up
    if (ObjectsPool::eagleObject == nullptr)
        return;
    Logger::instance() << "Level up!\n";
    _level++;
    globalVars::player1Level++;

    PlayerUpgrade::generateThreeRandomUpgradesForPlayer(_gameObject);
    globalVars::openLevelUpMenu = true;
}

void PlayerController::chooseUpgrade(int index)
{
    assert(index>=0 && index <= 3);
    assert(PlayerUpgrade::currentThreeRandomUpgrades.size() == 3);
    auto upgrade = PlayerUpgrade::currentThreeRandomUpgrades[index];
    assert(upgrade != nullptr);

    if (upgrade->category() == PlayerUpgrade::OneTimeBonus) {
        upgrade->onCollect(_gameObject);
        delete upgrade;
    } else {
        auto tp = upgrade->type();
        int lvl = hasLevelOfUpgrade(tp);
        // TODO magic names
        if (lvl > -1 && lvl < 3) {
            _collectedUpgrades[tp]->increaseLevel();
        } else {
            _collectedUpgrades[tp] = upgrade;
        }
    }

    // re-new all bonuses (like, armor protection etc) on every level-up
    for (auto it : _collectedUpgrades) {
        it.second->onCollect(_gameObject);
    }

    updateAppearance();
}

int PlayerController::hasLevelOfUpgrade(PlayerUpgrade::UpgradeType type) const
{
    if (_collectedUpgrades.find(type) == _collectedUpgrades.end())
        return -1;
    else
        return _collectedUpgrades.at(type)->currentLevel();
}

int PlayerController::numberOfUpgrades() const
{
    return _collectedUpgrades.size();
}

PlayerUpgrade *PlayerController::getUpgrade(int index) const
{
    assert(index < _collectedUpgrades.size());

    int i = 0;
    for (auto u : _collectedUpgrades) {
        if (i == index)
            return u.second;
        i++;
    }

    return nullptr;
}