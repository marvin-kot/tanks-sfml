#include "Controller.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "Damageable.h"
#include "DropGenerator.h"


Controller::Controller(GameObject *parent)
: _gameObject(parent)
{}

TankRandomController::TankRandomController(GameObject *parent, int speed, float timeoutSec)
: Controller(parent), _moveSpeed(speed), _actionTimeout(sf::seconds(timeoutSec)),distribution(0, 4)
{}

void TankRandomController::update()
{
    {
        using namespace globalVars;
        if (globalTimeFreeze) {
            if (globalFreezeClock.getElapsedTime() < sf::seconds(10)) {
                currMoveX = currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->move(currMoveX, currMoveY);
                return;
            }
            else
                globalTimeFreeze = false;
        }
    }

    if (_clock.getElapsedTime() > _actionTimeout) {
        _clock.restart();
        // change decision
        int dir = distribution(Utils::generator);
        switch (dir) {
            case 0: // stay / shoot
                currMoveX = currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->shoot();
                break;
            case 1: // left
                currMoveX = -_moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Left);
                _gameObject->setCurrentAnimation("left");
                _isMoving = true;
                break;
            case 2: // up
                currMoveY = -_moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Up);
                _gameObject->setCurrentAnimation("up");
                _isMoving = true;
                break;
            case 3: // right
                currMoveX = _moveSpeed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Right);
                _gameObject->setCurrentAnimation("right");
                _isMoving = true;
                break;
            case 4: // down
                currMoveY = _moveSpeed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Down);
                _gameObject->setCurrentAnimation("down");
                _isMoving = true;
                break;
        }
    }

    _gameObject->move(currMoveX, currMoveY);
}

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
    if (_invincible) {
        if (_invincibilityTimer.getElapsedTime() < sf::seconds(10))
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
        if (!wasPressed(SpacePressed))
            _pressedKeys[sf::Keyboard::Space] = _clock.getElapsedTime();
        setPressedFlag(SpacePressed, true);
    } else {
        setPressedFlag(SpacePressed, false);
        _pressedKeys.erase(sf::Keyboard::Space);
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

    switch (recentKey) {
        case sf::Keyboard::Space:
            _isMoving = false;
            if (_gameObject->shoot())
                SoundPlayer::instance().playShootSound();;
            break;
        case sf::Keyboard::Left:
            _gameObject->move(-moveSpeed, 0);
            _gameObject->setCurrentDirection(globalTypes::Left);
            _gameObject->setCurrentAnimation("left");
            _isMoving = true;
            break;
        case sf::Keyboard::Up:
            _gameObject->move(0, -moveSpeed);
            _gameObject->setCurrentDirection(globalTypes::Up);
            _gameObject->setCurrentAnimation("up");
            _isMoving = true;
            break;
        case sf::Keyboard::Right:
            _gameObject->move(moveSpeed, 0);
            _gameObject->setCurrentDirection(globalTypes::Right);
            _gameObject->setCurrentAnimation("right");
            _isMoving = true;
            break;
        case sf::Keyboard::Down:
            _gameObject->move(0, moveSpeed);
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

    Shootable *shootable = _gameObject->getComponent<Shootable>();
    assert(shootable != nullptr);

    switch (_powerLevel) {
        case 0:
            shootable->setActionTimeoutMs(Shootable::DefaultTimeoutMs);
            shootable->setDamage(Shootable::DefaultDamage);
            shootable->setBulletSpeed(Shootable::DefaultBulletSpeed);
            renderer->setSpriteSheetOffset(0, 0);
        case 1:
            shootable->setActionTimeoutMs(Shootable::DefaultTimeoutMs);
            shootable->setDamage(Shootable::DefaultDamage);
            shootable->setBulletSpeed(Shootable::DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 16);
            break;
        case 2:
            shootable->setActionTimeoutMs(Shootable::HalvedTimeoutMs);
            shootable->setDamage(Shootable::DefaultDamage);
            shootable->setBulletSpeed(Shootable::DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 32);
            break;
        case 3:
            shootable->setActionTimeoutMs(Shootable::HalvedTimeoutMs);
            shootable->setDamage(Shootable::DoubleDamage);
            shootable->setBulletSpeed(Shootable::DoubleBulletSpeed);
            renderer->setSpriteSheetOffset(0, 48);
            break;
    }
}

void PlayerController::setTemporaryInvincibility(int sec)
{
    _invincible = true;
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

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: Controller(obj), _direction(dir), _moveSpeed(spd), _damage(dmg)
{}

void BulletController::update()
{
    if (_direction == globalTypes::Left)
    {
        _gameObject->move(-_moveSpeed, 0);
        _gameObject->setCurrentAnimation("left");
    } else if (_direction == globalTypes::Up) {
        _gameObject->move(0, -_moveSpeed);
        _gameObject->setCurrentAnimation("up");
    } else if (_direction == globalTypes::Right) {
        _gameObject->move(_moveSpeed, 0);
        _gameObject->setCurrentAnimation("right");
    } else if (_direction == globalTypes::Down) {
        _gameObject->move(0, _moveSpeed);
        _gameObject->setCurrentAnimation("down");
    }
}

///// Spawner
SpawnController::SpawnController(GameObject *parent, std::string type, int timeout, int quantity)
: Controller(parent), _spawnableType(type), _spawnTimeout(sf::seconds(timeout)), _quantity(quantity)
{
    _gameObject->hide(true);
    _state = Starting;
}


void SpawnController::update()
{
    if (_quantity < 1) {
        _gameObject->_deleteme = true;
        return;
    }

    switch (_state) {
        case Waiting:
            if (_clock.getElapsedTime() > _spawnTimeout)
                _state = Starting;
            break;
        case Starting:
            Logger::instance() << "Starting";
            _gameObject->hide(false);
            _spawnAnimationclock.restart();
            _state = PlayingAnimation;
            break;
        case PlayingAnimation:
            if (_spawnAnimationclock.getElapsedTime() > _spawnAnimationTime && !_gameObject->collidesWithAnyObject())
                _state = CreateObject;
            break;
        case CreateObject:
            {
                GameObject * newNpc = createObject(_spawnableType);

                if (newNpc) {
                    newNpc->copyParentPosition(_gameObject);
                    ObjectsPool::addObject(newNpc);

                    if (_spawnBonusAtThisQuantity == _quantity) {
                        newNpc->appendFlags(GameObject::BonusOnHit);
                        newNpc->setDropGenerator(new DropGenerator(newNpc));
                    }
                    _quantity--;
                }

                _clock.restart();
                _gameObject->hide(true);
                _state = Waiting;
            }
            break;
    }
}

void SpawnController::setBonusSpawnWithProbability(int val)
{
    std::uniform_int_distribution<int> distribution(0, val);
    int rnd = distribution(Utils::generator);

    if (rnd > val) {
        // do not generate bonus
        _spawnBonusAtThisQuantity = -1;
        return;
    }

    std::uniform_int_distribution<int> tankDistr(1, _quantity);
    _spawnBonusAtThisQuantity = tankDistr(Utils::generator);
}

GameObject *SpawnController::createObject(std::string type)
{
    if (type == "npcBaseTank") {
        GameObject *enemy = new GameObject("npcBaseTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, 3, 0.75));

        return enemy;
    }

    if (type == "npcFastTank") {
        GameObject *enemy = new GameObject("npcFastTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, 4, 0.5));

        return enemy;
    }

    if (type == "npcArmorTank") {
        GameObject *enemy = new GameObject("npcArmorTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 3));
        enemy->setController(new TankRandomController(enemy, 2, 1));

        return enemy;
    }

    return nullptr;
}