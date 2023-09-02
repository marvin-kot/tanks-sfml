
#include "Damageable.h"
#include "EagleController.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"
#include "PersistentGameData.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "PlayerControllerClient.h"

#include <cmath>

static std::vector<int> xpNeededForLevelUp;


 void initXpLevelupNumbers()
 {
    std::vector<int> limits = { 300, 400 };
    for (int i = 2; i < 50 ; i++) {
        float coeff = 1;
        if (i<5)        coeff = 1.3;
        else if (i<8)   coeff = 1.2;
        //else if (i<9)  coeff = 1.1;
        else            coeff = 1.1;

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
    // if player has "atomic core" trait, kill all enemies instantly

    if (PlayerUpgrade::playerOwnedPerks.contains(PlayerUpgrade::KillAllOnDeath)) {
        std::unordered_set<GameObject *> objectsToKill = ObjectsPool::getObjectsByTypes({"npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcDoubleCannonArmorTank"});
        std::for_each(objectsToKill.cbegin(), objectsToKill.cend(), [](GameObject *obj) {
            obj->markForDeletion();
        });
    }

    resetXP();
}

void PlayerController::applyPerks()
{
    for (auto obj : PlayerUpgrade::availablePerkObjects) {
        if (PlayerUpgrade::playerOwnedPerks.contains(obj->type()))
            obj->onCollect(_gameObject);
    }
}

void PlayerController::removePerks()
{
    PlayerUpgrade::playerOwnedPerks.clear();
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
        if (_invincibilityTimer.getElapsedTime() < sf::milliseconds(_invincibilityTimeout)) {
            //_gameObject->visualEffect->copyParentPosition(_gameObject);
        } else {
            _invincible = false;
            delete _gameObject->visualEffect;
            _gameObject->visualEffect = nullptr;
            Damageable *d = _gameObject->getComponent<Damageable>();
            d->makeInvincible(false);
        }
    }
    bool action = false;

#ifdef SINGLE_APP
    PlayerControllerClient::instance().update(&_recentPlayerInput);
#endif

    if (_recentPlayerInput.shoot_request) {
        if (_gameObject->shoot())
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::Shoot, true);
    }

    //Logger::instance() << "speed: " << fSpeed << "/" << speed << "\n";
    globalTypes::Direction direction = static_cast<globalTypes::Direction>(_recentPlayerInput.direction_request);

    bool moved = false;
    if (direction != globalTypes::Direction::Unknown) {
        int speed = moveSpeedForCurrentFrame();
        assert( direction != globalTypes::Direction::Unknown );

        if (_gameObject->direction() != direction) {
            resetMoveStartTimer();
        }

        prepareMoveInDirection(direction, speed);
        if (_gameObject->move(_currMoveX, _currMoveY) > 0) {
            moved = true;
        } else {
            // try same direction but +1/-1 pixes aside
            moved = (trySqueeze() > 0);
        }

        if (!_prevMoved && moved)
            resetMoveStartTimer();

        _prevMoved = moved;

        _lastActionTime = _clock.getElapsedTime();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::TankMove, true);
        _gameObject->restartAnimation();
    } else {
        // check if on ice
        if (_gameObject->isOnIce()) {
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::iceSkid, true);
            if (_gameObject->move(_currMoveX, _currMoveY) == 0) {
                SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::TankStand, true);
                _gameObject->stopAnimation();
                _prevMoved = false;
            }
        } else {
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::TankStand, true);
            _gameObject->stopAnimation();
            _prevMoved = false;
        }
    }

    if (hasLevelOfUpgrade(PlayerUpgrade::BulletTank) == -1)
        return;

    if (_prevMoved && _moveStartClock.getElapsedTime() > sf::seconds(1)) {
        // if "bullet tank" ability - become bullet + invulnerable + blink
        if (Utils::currentFrame % 2 == 0)
            _gameObject->spriteRenderer->setOneFrameTintColor(sf::Color(0, 255, 0));
        _gameObject->damage = _gameObject->getComponent<Shootable>()->damage();
        setTemporaryInvincibility(globalConst::FixedFrameLength);
    } else {
        _gameObject->damage = 0;
    }
}

void PlayerController::resetMoveStartTimer()
{
    _moveStartClock.reset(true);
}

int PlayerController::trySqueeze()
{
    if (_currMoveX == 0) {
        if (_gameObject->move(1, _currMoveY) == 0)
            if (_gameObject->move(-1, _currMoveY) == 0)
                if (_gameObject->move(2, _currMoveY) == 0)
                    if (_gameObject->move(-2, _currMoveY) == 0)
                        if (_gameObject->move(3, _currMoveY) == 0)
                            return _gameObject->move(-3, _currMoveY);
    } else if (_currMoveY == 0) {
        if (_gameObject->move(_currMoveX, 1) == 0)
            if (_gameObject->move(_currMoveX, -1) == 0)
                if (_gameObject->move(_currMoveX, 2) == 0)
                    if (_gameObject->move(_currMoveX, -2) == 0)
                        if (_gameObject->move(_currMoveX, 3) == 0)
                            return _gameObject->move(_currMoveX, -3);
    }

    return 1;
}


void PlayerController::updateAppearance()
{
    assert(_gameObject != nullptr);
    assert(_gameObject->turret != nullptr);

    SpriteRenderer *baseRenderer = _gameObject->getComponent<SpriteRenderer>();
    SpriteRenderer *turretRenderer = _gameObject->turret->getComponent<SpriteRenderer>();
    assert(baseRenderer != nullptr);
    assert(turretRenderer != nullptr);

    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);

    int turretOffsetX = 0;
    int turretOffsetY = 0;

    switch (damageable->defence()) {
        case 0:
            baseRenderer->setSpriteSheetOffset(0, -16);
            turretOffsetX = 16;
            break;
        case 1:
            baseRenderer->setSpriteSheetOffset(0, 0);
            break;
        case 2:
            baseRenderer->setSpriteSheetOffset(0, 16);
            break;
        case 3:
            baseRenderer->setSpriteSheetOffset(0, 32);
            break;
        case 4:
            baseRenderer->setSpriteSheetOffset(0, 48);
            break;
        case 5:
            baseRenderer->setSpriteSheetOffset(0, 48);
            break;
    }

    Shootable *shootable = _gameObject->getComponent<Shootable>();
    if (shootable->bulletSpeed() > globalConst::DefaultPlayerBulletSpeed)
        turretOffsetY = 16;

    if (shootable->damage() > 1)
        turretOffsetY = 32;

    if (_4dirSet)
        turretOffsetY = 64;

    turretRenderer->setSpriteSheetOffset(turretOffsetX, turretOffsetY);

    baseRenderer->showAnimationFrame(0);
}

void PlayerController::onDamaged()
{
    if (!_invincible) {
        updateAppearance();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::debuff, true);
        _gameObject->getComponent<SpriteRenderer>()->setOneFrameTintColor(sf::Color::Red);
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
        cloud->setRenderer(new LoopAnimationSpriteRenderer(cloud, "cloud"), 4);
        _gameObject->visualEffect = cloud;
    }

}

void PlayerController::addXP(int val)
{
    _xp += val;
    Logger::instance() << "collect " << val << "xp\n";

    globalVars::player1XP += (val + val * _xpModifier / 100);

    if (_level >= xpNeededForLevelUp.size()) {
        Logger::instance() << "[ERROR] Level too big!\n";
        return;
    }

    if (_xp >= xpNeededForLevelUp[_level]) {
        levelUp();
    }
}

void PlayerController::resetXP()
{
    _xp = 0;
    _level = 0;

    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
        delete obj;
    }

    globalVars::player1XP = 0;
    globalVars::player1Level = 1;

    using namespace globalConst;
    Shootable *shootable = _gameObject->getComponent<Shootable>();
    assert(shootable != nullptr);
    delete shootable;
    _gameObject->setShootable(Shootable::createDefaultPlayerShootable(_gameObject));
    _4dirSet = false;

    Damageable *damageable = _gameObject->getComponent<Damageable>();
    damageable->setDefence(DefaultPlayerProtection);

    updateMoveSpeed(DefaultPlayerSpeed);

    updateAppearance();
}

void PlayerController::levelUp()
{
    // if we already lost, no sense to level up
    if (ObjectsPool::eagleObject == nullptr)
        return;
    Logger::instance() << "Level up!\n";
    _level++;
    globalVars::player1Level++;

    PlayerUpgrade::generateRandomUpgradesForPlayer(_gameObject);
    LevelUpPopupMenu::instance().open();

}

void PlayerController::chooseUpgrade(int index)
{
    assert(index>=0 && index < PlayerUpgrade::currentRandomUpgrades.size());

    auto upgrade = PlayerUpgrade::currentRandomUpgrades[index];
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
                removeUpgrade(PlayerUpgrade::PowerBullets);
                removeUpgrade(PlayerUpgrade::FastBullets);
            }
            if (tp == PlayerUpgrade::BulletTank) {
                removeUpgrade(PlayerUpgrade::TankArmor);
                removeUpgrade(PlayerUpgrade::TankSpeed);
            }
            if (tp == PlayerUpgrade::Rocket) {
                removeUpgrade(PlayerUpgrade::BonusEffectiveness);
                removeUpgrade(PlayerUpgrade::PowerBullets);
            }
            if (tp == PlayerUpgrade::MachineGun) {
                removeUpgrade(PlayerUpgrade::MoreBullets);
                removeUpgrade(PlayerUpgrade::FastBullets);
            }
            if (tp == PlayerUpgrade::FourDirectionBullets) {
                removeUpgrade(PlayerUpgrade::MoreBullets);
                removeUpgrade(PlayerUpgrade::PowerBullets);
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

void PlayerController::removeUpgrade(PlayerUpgrade::UpgradeType t)
{
    auto it2 = _collectedUpgrades.find(t);
    if (it2 != _collectedUpgrades.end()) {
        PlayerUpgrade *obj = (*it2).second;
        _collectedUpgrades.erase(it2);
        delete obj;
    }
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


void PlayerController::setFourDirectionTurret()
{
    if (_4dirSet) return;
    _4dirSet = true;

    // take old turret values
    auto oldShootable = _gameObject->getComponent<Shootable>();
    int actionTimeout = oldShootable->actionTimeoutMs();
    int damage = oldShootable->damage();
    int bulletSpeed = oldShootable->bulletSpeed();

    // delete old turret
    delete oldShootable;

    // set new turret
    auto newShootable = new FourDirectionShootable(_gameObject, bulletSpeed);
    //newShootable->setActionTimeoutMs(actionTimeout);
    newShootable->setDamage(damage);
    //newShootable->setBulletSpeed(bulletSpeed);

    _gameObject->setShootable(newShootable);
    updateAppearance();
}

void PlayerController::setRocketLauncher()
{
    if (_rocketSet) return;
    _rocketSet = true;

    // take old turret values
    auto oldShootable = _gameObject->getComponent<Shootable>();
    int actionTimeout = oldShootable->actionTimeoutMs();
    int damage = oldShootable->damage();
    int bulletSpeed = std::max(oldShootable->bulletSpeed(), globalConst::DefaultRocketSpeed);

    // delete old turret
    delete oldShootable;

    // set new turret
    auto newShootable = new RocketShootable(_gameObject);
    newShootable->setActionTimeoutMs(actionTimeout);
    newShootable->setDamage(damage);
    newShootable->setBulletSpeed(bulletSpeed);

    _gameObject->setShootable(newShootable);
    updateAppearance();
}