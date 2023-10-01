
#include "Collectable.h"
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
    std::vector<int> limits = { 300, 500 };
    for (int i = 2; i < 50 ; i++) {
        float coeff = 1;
        if (i<4)        coeff = 1.4;
        else if (i<6)        coeff = 1.3;
        else if (i<8)   coeff = 1.2;
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

    // if player has "war machine learning"
    if (PlayerUpgrade::playerOwnedPerks.contains(PlayerUpgrade::XpIncreaser)) {
        for (auto obj : PlayerUpgrade::availablePerkObjects) {
            if (obj->type() == PlayerUpgrade::XpIncreaser) {
                obj->increaseLevel();
            }
        }
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

void PlayerController::blinkIfParalyzed()
{
    if (_paralyzedForMs > 0) {
        if (_paralyzeClock.getElapsedTime() > sf::milliseconds(_paralyzedForMs))
        {
            _paralyzedForMs = 0;
            _gameObject->spriteRenderer->hide(false);
            _gameObject->turret->spriteRenderer->hide(false);
            return;
        } else if (_blinkClock.getElapsedTime() > sf::seconds(0.25)) {
                _blinkClock.reset(true);
                _blink = !_blink;
                _gameObject->spriteRenderer->hide(_blink);
                _gameObject->turret->spriteRenderer->hide(_blink);
        }
    }
}

void PlayerController::update()
{
    checkForGamePause();
    if (_pause) return;
    assert(_gameObject != nullptr);

    if (_invincible && _invincibilityTimer.getElapsedTime() >= sf::milliseconds(_invincibilityTimeout)) {
        _invincible = false;
        delete _gameObject->visualEffect;
        _gameObject->visualEffect = nullptr;
        Damageable *d = _gameObject->getComponent<Damageable>();
        d->makeInvincible(false);
    }

    blinkIfParalyzed();

    bool action = false;

#ifdef SINGLE_APP
    PlayerControllerClient::instance().update(&_recentPlayerInput);
#endif

    if (_recentPlayerInput.shoot_request) {
        if (_gameObject->shoot()) {
            auto shootable = _gameObject->getComponent<Shootable>();
            assert(shootable != nullptr);
            SoundPlayer::instance().enqueueSound(shootable->shootSound(), true);
            if (_4dirSet)
                _gameObject->turret->spriteRenderer->setCurrentAnimation("upgrade-4dir-shot", true);
            else if (_rocketSet)
                _gameObject->turret->spriteRenderer->setCurrentAnimation("upgrade-rocket-shot", true);
            else if (_upgradePower)
                _gameObject->turret->spriteRenderer->setCurrentAnimation("upgrade-power-shot", true);
            else if (_upgradeSpeed)
                _gameObject->turret->spriteRenderer->setCurrentAnimation("upgrade-speed-shot", true);
            else
                _gameObject->turret->spriteRenderer->setCurrentAnimation("default-shot", true);
        }
    } else if (_recentPlayerInput.weapon2_request) {
        if (_gameObject->useSecondWeapon()) {
            SoundPlayer::instance().enqueueSound(SoundPlayer::SetLandmine, true);
        } else
            SoundPlayer::instance().enqueueSound(SoundPlayer::RejectLandmine, true);
    }

    globalTypes::Direction direction = (_paralyzedForMs == 0)
            ? static_cast<globalTypes::Direction>(_recentPlayerInput.direction_request)
            : globalTypes::Direction::Unknown;

    bool moved = false;
    if (direction != globalTypes::Direction::Unknown) {
        int speed = moveSpeedForCurrentFrame();
        assert( direction != globalTypes::Direction::Unknown );
        if (_gameObject->direction() != direction) {
            resetMoveStartTimer();
        }

        prepareMoveInDirection(direction, speed);
        moved = (_gameObject->move(_currMoveX, _currMoveY) > 0) ? true : (trySqueeze() > 0);

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

    int ramLevel = hasLevelOfUpgrade(PlayerUpgrade::RamMachine);
    if (ramLevel > -1) {
        if (_prevMoved && _moveStartClock.getElapsedTime() > sf::seconds(0.1)) {
            _gameObject->damage = ramLevel+1;
            if (Utils::currentFrame % 2 == 0) {
                assert (_gameObject->spriteRenderer);
                _gameObject->spriteRenderer->setOneFrameTintColor(sf::Color(200, 200, 255));
            }
        } else
            _gameObject->damage = 0;
    }





    if (hasLevelOfUpgrade(PlayerUpgrade::BulletTank) > -1) {
        if (_prevMoved && _moveStartClock.getElapsedTime() > sf::seconds(0.9)) {
            // if "bullet tank" ability - become bullet + invulnerable + blink
            if (Utils::currentFrame % 2 == 0) {
                assert (_gameObject->spriteRenderer);
                assert (_gameObject->turret);
                assert (_gameObject->turret->spriteRenderer);
                _gameObject->spriteRenderer->setOneFrameTintColor(sf::Color(0, 255, 0));
                _gameObject->turret->spriteRenderer->setOneFrameTintColor(sf::Color(0, 255, 0));
            }
            _gameObject->damage = _gameObject->getComponent<Shootable>()->damage();
            setTemporaryInvincibility(globalConst::FixedFrameLength);
        } else {
            _gameObject->damage = 0;
        }
    }
}

void PlayerController::resetMoveStartTimer()
{
    _moveStartClock.reset(true);
}

int PlayerController::trySqueeze()
{
    if (_currMoveX == 0) {
        if (_gameObject->move(2, _currMoveY) == 0)
            if (_gameObject->move(-2, _currMoveY) == 0)
                if (_gameObject->move(4, _currMoveY) == 0)
                    if (_gameObject->move(-4, _currMoveY) == 0)
                        if (_gameObject->move(6, _currMoveY) == 0)
                            return _gameObject->move(-6, _currMoveY);
    } else if (_currMoveY == 0) {
        if (_gameObject->move(_currMoveX, 2) == 0)
            if (_gameObject->move(_currMoveX, -2) == 0)
                if (_gameObject->move(_currMoveX, 4) == 0)
                    if (_gameObject->move(_currMoveX, -4) == 0)
                        if (_gameObject->move(_currMoveX, 6) == 0)
                            return _gameObject->move(_currMoveX, -6);
    }

    return 1;
}


void PlayerController::updateAppearance()
{
    assert(_gameObject != nullptr);
    assert(_gameObject->turret != nullptr);

    SpriteRenderer *baseRenderer = _gameObject->getComponent<SpriteRenderer>();
    if (baseRenderer == nullptr) return;

    SpriteRenderer *turretRenderer = _gameObject->turret->getComponent<SpriteRenderer>();
    if (turretRenderer == nullptr) return;

    assert(baseRenderer != nullptr);
    assert(turretRenderer != nullptr);

    Damageable *damageable = _gameObject->getComponent<Damageable>();
    if (damageable == nullptr) return;
    assert(damageable != nullptr);

    int turretOffsetX = 0;
    int turretOffsetY = 0;

    using namespace globalConst;
    bool damaged = false;

    switch (damageable->defence()) {
        case 0:
            baseRenderer->setCurrentAnimation("damaged");
            damaged = true;
            break;
        case 1:
            baseRenderer->setCurrentAnimation("default");
            break;
        case 2:
            baseRenderer->setCurrentAnimation("protection-1");
            break;
        case 3:
            baseRenderer->setCurrentAnimation("protection-1");
            break;
        case 4:
            baseRenderer->setCurrentAnimation("protection-2");
            break;
        case 5:
            baseRenderer->setCurrentAnimation("protection-2");
            break;
    }

    Shootable *shootable = _gameObject->getComponent<Shootable>();
    if (shootable == nullptr) return;
    assert(shootable != nullptr);

    std::string turretAnimation = "default";

    if (shootable->bulletSpeed() > globalConst::DefaultPlayerBulletSpeed) {
        _upgradeSpeed = true;
        turretAnimation = "upgrade-speed";
    } else {
        _upgradeSpeed = false;
    }

    if (shootable->damage() > 1) {
        _upgradePower = true;
        turretAnimation = "upgrade-power";
    } else
        _upgradePower = false;

    if (_4dirSet)
        turretAnimation = "upgrade-4dir";

    if (_rocketSet)
        turretAnimation = "upgrade-rocket";

    if (damaged)
        turretAnimation += "-damaged";

    turretRenderer->setCurrentAnimation(turretAnimation);

    baseRenderer->showAnimationFrame(0);
}

void PlayerController::onDamaged()
{
    if (!_invincible) {
        updateAppearance();
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::GetDamage, true);
        _gameObject->getComponent<SpriteRenderer>()->setOneFrameTintColor(sf::Color::Red);
        setTemporaryInvincibility(750);
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
    int xpIncrease = (val + val * _xpModifier / 100);

    globalVars::player1XP += xpIncrease;
    _xp += xpIncrease;

    if (_level >= xpNeededForLevelUp.size()) {
        Logger::instance() << "[ERROR] Level too big!\n";
        return;
    }

    if (_xp >= xpNeededForLevelUp[_level])
        levelUp();
}

void PlayerController::dropSkull()
{
    GameObject *collectable = new GameObject("skullCollectable");
    collectable->setFlags(GameObject::CollectableBonus | GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
    collectable->setRenderer(new SpriteRenderer(collectable), 4);
    collectable->setController(new SkullController(collectable, 12000));

    SkullCollectable *skull = new SkullCollectable(collectable);
    skull->xp = _xp;
    skull->level = _level;
    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ++it) {
        PlayerUpgrade *obj = (*it).second;
        skull->playerUpgrades.push_back(obj);
    }

    collectable->setCollectable(skull);

    collectable->copyParentPosition(_gameObject);
    ObjectsPool::addObject(collectable);
}

void PlayerController::resetXP()
{
    _xp = 0;
    _level = 0;

    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
        if (globalVars::player1Lives < 0)
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
    _speedDebuff = 0;
    setMoveSpeed(DefaultPlayerSpeed);

    updateAppearance();
}

void PlayerController::restoreLevelAndUpgrades(SkullCollectable *skull)
{
    Logger::instance() << "rectore level ";
    globalVars::player1XP = _xp = skull->xp;
    globalVars::player1Level = _level = skull->level;

    // delete existing upgrades
    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
    }

    for (PlayerUpgrade *upgrade : skull->playerUpgrades) {
        auto type = upgrade->type();
        _collectedUpgrades[type] = upgrade;
    }

    applyUpgrades();
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
            if (tp == PlayerUpgrade::ReloadOnKill) {
                removeUpgrade(PlayerUpgrade::FastReload);
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
    resetCalculatedReloadDebuff();
    resetCalculatedSpeedDebuff();
    _moveSpeed = globalConst::DefaultPlayerSpeed;

    for (auto it : _collectedUpgrades) {
        it.second->onCollect(_gameObject);
    }

    applyCalculatedReloadDebuff();
    applyCalculatedSpeedDebuff();

    // restore basic defence if needed
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);
    if (damageable->defence() < globalConst::DefaultPlayerProtection)
        damageable->setDefence(globalConst::DefaultPlayerProtection);
    // restore ammunition
    Shootable * shootable = _gameObject->getComponent<Shootable>();
    shootable->resetBullets();

    updateAppearance();
}

void PlayerController::restoreProtection()
{
}

int PlayerController::hasLevelOfUpgrade(PlayerUpgrade::UpgradeType type) const
{
    return _collectedUpgrades.contains(type) ? _collectedUpgrades.at(type)->currentLevel() : -1;
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
    int shootTimeout = oldShootable->shootTimeoutMs();
    int reloadTimeout = oldShootable->reloadTimeoutMs();
    int damage = oldShootable->damage();
    int bulletSpeed = oldShootable->bulletSpeed();
    int maxBullets = oldShootable->maxBullets();

    // delete old turret
    delete oldShootable;

    // set new turret
    auto newShootable = new FourDirectionShootable(_gameObject, bulletSpeed, maxBullets);
    newShootable->setDamage(damage);
    newShootable->setShootTimeoutMs(shootTimeout);
    newShootable->setReloadTimeoutMs(reloadTimeout);

    _gameObject->setShootable(newShootable);
    updateAppearance();
}

void PlayerController::setRocketLauncher()
{
    if (_rocketSet) return;
    _rocketSet = true;

    // take old turret values
    auto oldShootable = _gameObject->getComponent<Shootable>();
    int actionTimeout = oldShootable->shootTimeoutMs();
    int reloadTimeout = oldShootable->reloadTimeoutMs();
    int damage = oldShootable->damage();
    int bulletSpeed = std::max(oldShootable->bulletSpeed(), globalConst::DefaultRocketSpeed);
    int maxBullets = oldShootable->maxBullets();

    // delete old turret
    delete oldShootable;

    // set new turret
    auto newShootable = new RocketShootable(_gameObject, maxBullets);
    newShootable->setDamage(damage);
    newShootable->setShootTimeoutMs(actionTimeout);
    newShootable->setReloadTimeoutMs(reloadTimeout);
    newShootable->setBulletSpeed(bulletSpeed);

    _gameObject->setShootable(newShootable);
    updateAppearance();
}


void PlayerController::setMoveSpeed(int speed) {
    _moveSpeed = speed;
}

void PlayerController::applyCalculatedSpeedDebuff()
{
    _moveSpeed -= _moveSpeed * _speedDebuff / 100;
}


void PlayerController::applyCalculatedReloadDebuff()
{
    assert(_gameObject != nullptr);
    auto shootable = _gameObject->getComponent<Shootable>();
    assert(shootable != nullptr);

    int newShootTimeout = globalConst::PlayerShootTimeoutMs + globalConst::PlayerShootTimeoutMs * _shootReloadDebuff / 100;
    shootable->setShootTimeoutMs(newShootTimeout);
}

void PlayerController::onKillEnemy(GameObject *enemy)
{
    (void *)enemy;

    if (hasLevelOfUpgrade(PlayerUpgrade::ReloadOnKill) != -1) {
        auto shootable = _gameObject->getComponent<Shootable>();
        assert(shootable != nullptr);
        if (shootable->bullets() < shootable->maxBullets()) {
            shootable->resetBullets();
            SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::FullReload, true);
        }
    }
}


