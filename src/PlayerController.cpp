#include "PlayerController.h"

#include "Damageable.h"
#include "EagleController.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"
#include "Shootable.h"
#include "SoundPlayer.h"
#include "Utils.h"

#include <cmath>

static std::vector<int> xpNeededForLevelUp;


 void initXpLevelupNumbers()
 {
    std::vector<int> limits = {400, 600, };
    for (int i = 2; i < 20 ; i++) {
        float coeff = i<6 ? 1.4 : i<10 ? 1.3 : 1.2;
        int val = ((int)(limits[i-1] * coeff) / 100) * 100;
        limits.push_back(val);
    }

    xpNeededForLevelUp.clear();

    Logger::instance() << "XP limits for level ups:\n";
    xpNeededForLevelUp.push_back(limits[0]);
    for (int i = 1; i < limits.size() ; i++) {
        xpNeededForLevelUp.push_back( xpNeededForLevelUp[i-1] + limits[i] );
        Logger::instance() << "    l." << i << " - " << xpNeededForLevelUp[i-1] << "\n";
    }
 }



PlayerController::PlayerController(GameObject *obj)
: Controller(obj, globalConst::DefaultPlayerSpeed)
{
    _clock.reset(true);
    resetXP();
    initXpLevelupNumbers();
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
        if (_invincibilityTimer.getElapsedTime() < sf::milliseconds(_invincibilityTimeout))
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

    //Logger::instance() << "speed: " << fSpeed << "/" << speed << "\n";
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
        int speed = moveSpeedForCurrentFrame();
        assert( direction != globalTypes::Direction::Unknown );
        prepareMoveInDirection(direction, speed);
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
        if (_gameObject->move(1, _currMoveY) == 0)
            if (_gameObject->move(-1, _currMoveY) == 0)
                if (_gameObject->move(2, _currMoveY) == 0)
                    _gameObject->move(-2, _currMoveY);
    } else if (_currMoveY == 0) {
        if (_gameObject->move(_currMoveX, 1) == 0)
            if (_gameObject->move(_currMoveX, -1) == 0)
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
            shootable->setBulletSpeed(DefaultPlayerBulletSpeed);
            renderer->setSpriteSheetOffset(0, 0);
        case 1:
            shootable->resetLevel();
            shootable->setDamage(DefaultDamage);
            shootable->setBulletSpeed(DefaultPlayerBulletSpeed);
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 2:
            shootable->resetLevel();
            shootable->increaseLevel();
            shootable->setDamage(DefaultDamage);
            shootable->setBulletSpeed(DoublePlayerBulletSpeed);
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 3:
            shootable->resetLevel();
            shootable->increaseLevel();
            shootable->setDamage(DoubleDamage);
            shootable->setBulletSpeed(DoublePlayerBulletSpeed);
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
            renderer->setSpriteSheetOffset(0, 256);
            break;
        case 1:
            renderer->setSpriteSheetOffset(0, 0);
            break;
        case 2:
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 3:
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 4:
            renderer->setSpriteSheetOffset(0, 48);
            break;
        case 5:
            renderer->setSpriteSheetOffset(0, 48);
            break;
    }

    renderer->showAnimationFrame(0);
}

void PlayerController::onDamaged()
{
    if (!_invincible) {
        updateAppearance();
        SoundPlayer::instance().playDebuffSound();
        _gameObject->getComponent<SpriteRenderer>()->setOneFrameSpriteSheetOffset(128, 128);
        setTemporaryInvincibility(500);
    }
}

void PlayerController::setTemporaryInvincibility(int msec)
{
    _invincible = true;
    _invincibilityTimeout = msec;
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

void PlayerController::addXP(int val)
{
    _xp += val;
    Logger::instance() << "collect " << val << "xp\n";
    globalVars::player1XP += (val + val*_xpModifier/100);
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
    LevelUpPopupMenu::instance().open();

}

void PlayerController::chooseUpgrade(int index)
{
    assert(index>=0 && index <= 3);
    assert(PlayerUpgrade::currentThreeRandomUpgrades.size() == 3);
    auto upgrade = PlayerUpgrade::currentThreeRandomUpgrades[index];
    assert(upgrade != nullptr);

    assert(ObjectsPool::eagleObject != nullptr);
    auto eagleController = ObjectsPool::eagleObject->getComponent<EagleController>();
    assert(eagleController != nullptr);

    switch (upgrade->category())
    {
        case PlayerUpgrade::OneTimeBonus:
            upgrade->onCollect(_gameObject);
            delete upgrade;
            break;
        case PlayerUpgrade::TankUpgrade:
        {
            auto tp = upgrade->type();
            int lvl = hasLevelOfUpgrade(tp);
            // TODO magic names
            if (lvl > -1 && lvl < globalConst::MaxUpgradeLevel) {
                _collectedUpgrades[tp]->increaseLevel();
            } else {
                _collectedUpgrades[tp] = upgrade;
            }

            // specific cases
            if (tp == PlayerUpgrade::PiercingBullets) {
                // delete power bullet upgrade
                auto it1 = _collectedUpgrades.find(PlayerUpgrade::PowerBullets);
                if (it1 != _collectedUpgrades.end()) {
                    PlayerUpgrade *obj = (*it1).second;
                    _collectedUpgrades.erase(it1);
                    delete obj;
                }

                // delete fast bullet upgrade
                auto it2 = _collectedUpgrades.find(PlayerUpgrade::FastBullets);
                if (it2 != _collectedUpgrades.end()) {
                    PlayerUpgrade *obj = (*it2).second;
                    _collectedUpgrades.erase(it2);
                    delete obj;
                }

                // remove from available to generate as well
                //PlayerUpgrade::removeFromAvailable(PlayerUpgrade::PowerBullets);
                //PlayerUpgrade::removeFromAvailable(PlayerUpgrade::FastBullets);
            }
            break;
        }
        case PlayerUpgrade::BaseUpgrade:
            eagleController->upgrade(upgrade);
            break;
    }

    // re-apply all bonuses (like, armor protection etc) on every level-up
    applyUpgrades();
    eagleController->applyUpgrades();
}

void PlayerController::applyUpgrades()
{
    for (auto it : _collectedUpgrades) {
        it.second->onCollect(_gameObject);
    }

    // restore basic defence if needed
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);
    if (damageable->defence() < globalConst::DefaultPlayerProtection)
        damageable->setDefence(globalConst::DefaultPlayerProtection);

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