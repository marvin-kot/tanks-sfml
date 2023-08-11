#include "Controller.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "Logger.h"


Controller::Controller(GameObject *parent)
: _gameObject(parent)
{}

/////

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
        int speed = ((int)(_moveSpeed * Utils::lastFrameTime.asSeconds()) << 1) >> 1;
        switch (dir) {
            case 0: // stay / shoot
                currMoveX = currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->shoot();
                break;
            case 1: // left
                currMoveX = -speed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Left);
                _gameObject->setCurrentAnimation("left");
                _isMoving = true;
                break;
            case 2: // up
                currMoveY = -speed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Up);
                _gameObject->setCurrentAnimation("up");
                _isMoving = true;
                break;
            case 3: // right
                currMoveX = speed; currMoveY = 0;
                _gameObject->setCurrentDirection(globalTypes::Right);
                _gameObject->setCurrentAnimation("right");
                _isMoving = true;
                break;
            case 4: // down
                currMoveY = speed; currMoveX = 0;
                _gameObject->setCurrentDirection(globalTypes::Down);
                _gameObject->setCurrentAnimation("down");
                _isMoving = true;
                break;
        }
    }

    _gameObject->move(currMoveX, currMoveY);
}

/////

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: Controller(obj), _direction(dir), _moveSpeed(spd), _damage(dmg)
{}

void BulletController::update()
{
    int speed = (int)((float)_moveSpeed * Utils::lastFrameTime.asSeconds());
    if (_direction == globalTypes::Left)
    {
        _gameObject->move(-speed, 0);
        _gameObject->setCurrentAnimation("left");
    } else if (_direction == globalTypes::Up) {
        _gameObject->move(0, -speed);
        _gameObject->setCurrentAnimation("up");
    } else if (_direction == globalTypes::Right) {
        _gameObject->move(speed, 0);
        _gameObject->setCurrentAnimation("right");
    } else if (_direction == globalTypes::Down) {
        _gameObject->move(0, speed);
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
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed, 0.75));

        return enemy;
    }

    if (type == "npcFastTank") {
        GameObject *enemy = new GameObject("npcFastTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed*4/3, 0.5));

        return enemy;
    }

    if (type == "npcArmorTank") {
        GameObject *enemy = new GameObject("npcArmorTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 3));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed*3/4, 1));

        return enemy;
    }

    return nullptr;
}




PlayerSpawnController::PlayerSpawnController(GameObject *parent, int lives, int powerLevel)
: Controller(parent), _lives(lives), _initialPowerLevel(powerLevel)
{
    _gameObject->hide(true);
    _state = Starting;
}

GameObject * PlayerSpawnController::createObject()
{
    Logger::instance() << "Creating player...";
    GameObject *pc = new GameObject("player");
    pc->setShootable(new Shootable(pc));
    pc->setFlags(GameObject::Player | GameObject::BulletKillable);
    pc->setRenderer(new SpriteRenderer(pc));
    pc->setDamageable(new Damageable(pc, 1));
    // must be done after creating Damageable
    pc->setController(new PlayerController(pc));
    pc->setCurrentDirection(globalTypes::Up);

    return pc;
}

void PlayerSpawnController::update()
{
    if (_lives < 1) {
        _gameObject->_deleteme = true;
        return;
    }

    switch (_state) {
        case Waiting:
            if (ObjectsPool::playerObject == nullptr) {
                _initialPowerLevel = 0; // as player was killed, its power resets to 0
                _state = Starting;
            }
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
                GameObject * newPc = createObject();

                if (newPc) {
                    newPc->setParent(_gameObject);
                    newPc->copyParentPosition(_gameObject);
                    ObjectsPool::addObject(newPc);
                    ObjectsPool::playerObject = newPc;

                    auto controller = newPc->getComponent<PlayerController>();
                    controller->setTemporaryInvincibility(3);
                    for (int i=0; i<_initialPowerLevel; i++)
                        controller->increasePowerLevel(true);
                    _lives--;
                }

                _clock.restart();
                _gameObject->hide(true);
                _state = Waiting;
            }
            break;
    }
}

void PlayerSpawnController::appendLife()
{
    _lives++;
}