#include "Damageable.h"
#include "EagleController.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "ObjectsPool.h"
#include "PersistentGameData.h"
#include "PlayerController.h"
#include "MapCreator.h"
#include "SoundPlayer.h"
#include "SpriteRenderer.h"
#include "Utils.h"

using namespace globalTypes;
using namespace sf;

constexpr int subtileSize = globalConst::spriteOriginalSizeX/2;
constexpr int subtileCenter = subtileSize/2;
constexpr int wallOffset = subtileSize + subtileCenter;

static const std::map<EagleWallDirection, sf::Vector2i> dirOffsets = {
    {DownLeft, Vector2i(-wallOffset, wallOffset)},
    {LeftLeftDown, Vector2i(-wallOffset, subtileCenter)},
    {LeftLeftUp, Vector2i(-wallOffset, -subtileCenter)},
    {UpLeft, Vector2i(-wallOffset, -wallOffset)},
    {UpUpLeft, Vector2i(-subtileCenter, -wallOffset)},
    {UpUpRight, Vector2i(subtileCenter, -wallOffset)},
    {UpRight, Vector2i(wallOffset, -wallOffset)},
    {RightRightUp, Vector2i(wallOffset, -subtileCenter)},
    {RightRightDown, Vector2i(wallOffset, subtileCenter)},
    {DownRight, Vector2i(wallOffset, wallOffset)},
    {DownDownRight, Vector2i(subtileCenter, wallOffset)},
    {DownDownLeft, Vector2i(-subtileCenter, wallOffset)}
};

static const std::map<EagleWallDirection, std::string> dirWallTypes = {
    {DownLeft, "brickWall1x2"},
    {LeftLeftDown, "brickWall1x2"},
    {LeftLeftUp, "brickWall1x1"},
    {UpLeft, "brickWall1x1"},
    {UpUpLeft, "brickWall1x1"},
    {UpUpRight, "brickWall2x1"},
    {UpRight, "brickWall2x1"},
    {RightRightUp, "brickWall2x1"},
    {RightRightDown, "brickWall2x2"},
    {DownRight, "brickWall2x2"},
    {DownDownRight, "brickWall2x2"},
    {DownDownLeft, "brickWall1x2"}
};

EagleController::EagleController(GameObject *obj)
: Controller(obj, 0), _state(Starting), _invincibilityAfterDamageTimeout(750),_invincible(false)
{}

EagleController::~EagleController()
{
    for (auto it = _collectedUpgrades.begin(); it != _collectedUpgrades.end(); ) {
        PlayerUpgrade *obj = (*it).second;
        it = _collectedUpgrades.erase(it);
        delete obj;
    }
}

constexpr int initialWallsBuildTimeout = 100;

void EagleController::update()
{
    checkForGamePause();
    if (_pause) return;

    if (_invincible) {
        if (_invincibilityTimer.getElapsedTime() < sf::milliseconds(_invincibilityAfterDamageTimeout)) {
            _gameObject->visualEffect->copyParentPosition(_gameObject);
        } else {
            _invincible = false;
            delete _gameObject->visualEffect;
            _gameObject->visualEffect = nullptr;
            Damageable *d = _gameObject->getComponent<Damageable>();
            d->makeInvincible(false);
        }
    }


    switch (_state) {
        case Starting:
            _currentBuildDirection = globalTypes::EagleWallDirection::DownLeft;
            _state = BuildingWalls;
            _clock.reset(true);
            _rebuildTimeouts.push(initialWallsBuildTimeout);
            break;
        case BuildingWalls:
            assert(!_rebuildTimeouts.empty());
            if (_clock.getElapsedTime() > sf::milliseconds(_rebuildTimeouts.top())) {
                _clock.reset(true);
                if (false == ObjectsPool::getEagleWalls().contains(_currentBuildDirection)) {
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::BuildWall, true);

                    const int thisX = _gameObject->position().x;
                    const int thisY = _gameObject->position().y;
                    const auto offset = dirOffsets.at(_currentBuildDirection);
                    //Logger::instance() << "build dir " << (int)_currentBuildDirection << " coord " << thisX + offset.x << " " << thisY + offset.y << "\n";
                    GameObject *obj = MapCreator::buildObject("brickWall-segment"/*dirWallTypes.at(_currentBuildDirection)*/);
                    assert(obj != nullptr);
                    obj->setSize(globalConst::spriteOriginalSizeX/2, globalConst::spriteOriginalSizeY/2);
                    obj->setPosition(
                    thisX + offset.x, thisY + offset.y);
                    ObjectsPool::addEagleWall(_currentBuildDirection, obj);
                }

                _currentBuildDirection = static_cast<EagleWallDirection>((int)_currentBuildDirection + 1);
                if (_currentBuildDirection == EagleWallDirection::MaxDirection) {
                    if (_rebuildTimeouts.size() > 1 || !_isSlowRepairMode)
                        _rebuildTimeouts.pop();
                    _state = Waiting;
                }
            }
            break;
        case Waiting:
            break;
    }

}

void EagleController::fastRepairWalls(int timeout)
{
    _currentBuildDirection = globalTypes::EagleWallDirection::DownLeft;
    _rebuildTimeouts.push(timeout);
    _state = BuildingWalls;
}

void EagleController::setSlowRepairMode(int timeout)
{

    _currentBuildDirection = globalTypes::EagleWallDirection::DownLeft;
    _rebuildTimeouts.push(timeout);
    _state = BuildingWalls;
    _isSlowRepairMode = true;
}

void EagleController::removeUpgrade(PlayerUpgrade::UpgradeType t)
{
    auto it2 = _collectedUpgrades.find(t);
    if (it2 != _collectedUpgrades.end()) {
        PlayerUpgrade *obj = (*it2).second;
        _collectedUpgrades.erase(it2);
        delete obj;
    }
}

void EagleController::upgrade(PlayerUpgrade *upgrade)
{
    assert(upgrade != nullptr);

    auto tp = upgrade->type();
    int lvl = hasLevelOfUpgrade(tp);
    // TODO magic names
    if (lvl > -1 && lvl < 3) {
        _collectedUpgrades[tp]->increaseLevel();
    } else {
        _collectedUpgrades[tp] = upgrade;
    }

    if (tp == PlayerUpgrade::BaseRestoreOnDamage) {
        removeUpgrade(PlayerUpgrade::RepairWalls);
        removeUpgrade(PlayerUpgrade::BaseInvincibility);
    }
}

void EagleController::applyUpgrades()
{
    // re-new all bonuses (like, armor protection etc)
    for (auto it : _collectedUpgrades) {
        it.second->onCollect(_gameObject);
    }

    // restore basic defence if needed
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);
    if (damageable->defence() < globalConst::DefaultBaseProtection)
        damageable->setDefence(globalConst::DefaultBaseProtection);

    updateAppearance();
}

void EagleController::updateAppearance()
{
    SpriteRenderer *renderer = _gameObject->getComponent<SpriteRenderer>();
    assert(renderer != nullptr);
    Damageable *damageable = _gameObject->getComponent<Damageable>();
    assert(damageable != nullptr);

    using namespace globalConst;

    switch (damageable->defence()) {
        case 0:
            renderer->setCurrentAnimation("damaged");
            break;
        case 1:
            renderer->setCurrentAnimation("default");
            break;
        case 2:
            renderer->setCurrentAnimation("upgrade-1");
            break;
        case 3:
            renderer->setCurrentAnimation("upgrade-2");
            break;
        case 4:
            renderer->setCurrentAnimation("upgrade-2");
            break;
        case 5:
            renderer->setCurrentAnimation("upgrade-3");
            break;
    }

    renderer->showAnimationFrame(0);
}

int EagleController::hasLevelOfUpgrade(PlayerUpgrade::UpgradeType type) const
{
    if (_collectedUpgrades.find(type) == _collectedUpgrades.end())
        return -1;
    else
        return _collectedUpgrades.at(type)->currentLevel();
}

int EagleController::numberOfUpgrades() const
{
    return _collectedUpgrades.size();
}

PlayerUpgrade *EagleController::getUpgrade(int index) const
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

void EagleController::setTempInvincibilityAfterDamage(int timeout)
{
    _invincibilityAfterDamageTimeout = timeout;
}

void EagleController::onDamaged()
{
    updateAppearance();
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::GetDamage, true);

    Damageable *dmg = _gameObject->getComponent<Damageable>();

    if (_invincibilityAfterDamageTimeout > 0) {
        _invincible = true;
        _invincibilityTimer.reset(true);
        Damageable *dmg = _gameObject->getComponent<Damageable>();
        dmg->makeInvincible(true);

        if (_gameObject->visualEffect == nullptr) {
            GameObject *cloud = new GameObject(_gameObject, "cloud");
            cloud->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
            cloud->setRenderer(new LoopAnimationSpriteRenderer(cloud, "cloud"), 4);
            _gameObject->visualEffect = cloud;
        }
    }

    if (ObjectsPool::playerObject == nullptr)
        return;

    PlayerController *controller = ObjectsPool::playerObject->getComponent<PlayerController>();

    if (dmg->isDestroyed()) {
        if (PlayerUpgrade::playerOwnedPerks.contains(PlayerUpgrade::SacrificeLifeForBase)) {
            ObjectsPool::playerObject->markForDeletion(); // kill player
            dmg->setDefence(globalConst::DefaultBaseProtection);
            updateAppearance();
            fastRepairWalls(100);

            // can be used only once per run
            PlayerUpgrade::playerOwnedPerks.erase(PlayerUpgrade::SacrificeLifeForBase);
        } else {
            // eagle is dead - run is finished
        }
    } else {
        Utils::triggerScreenShake(10.0f, 15);
        HUD::instance().onBaseDamaged();
        if (hasLevelOfUpgrade(PlayerUpgrade::BaseRestoreOnDamage) > -1)
            fastRepairWalls(100);

    }
}


GameObject *EagleController::onDestroyed()
{
    ObjectsPool::eagleObject = nullptr;
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bigExplosion, true);
    Utils::triggerScreenShake(20.0f, 20);
    return ExplosionController::createBigExplosion(_gameObject, false);
}