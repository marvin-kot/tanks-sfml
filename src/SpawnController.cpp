#include "Controller.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "Shootable.h"
#include "Utils.h"

SpawnController::SpawnController(GameObject *parent, std::string type, int timeout, int quantity)
: Controller(parent, 0), _spawnableType(type), _spawnTimeout(sf::seconds(timeout)), _quantity(quantity)
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

                    /*if (_spawnBonusAtThisQuantity == _quantity) {
                        newNpc->appendFlags(GameObject::BonusOnHit);
                    }*/
                    //newNpc->setDropGenerator(new DropGenerator(newNpc));
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
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        enemy->setShootable(new PlayerShootable(enemy, 0));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed, 0.75));
        enemy->setDropGenerator(new DropGenerator(enemy, 100));

        return enemy;
    }

    if (type == "npcFastTank") {
        GameObject *enemy = new GameObject("npcFastTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::HalvedTimeoutMs));
        enemy->setShootable(new PlayerShootable(enemy, 0));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed*4/3, 0.5));
        enemy->setDropGenerator(new DropGenerator(enemy, 300));

        return enemy;
    }

    if (type == "npcArmorTank") {
        GameObject *enemy = new GameObject("npcArmorTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        enemy->setShootable(new PlayerShootable(enemy, 0));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy));
        enemy->setDamageable(new Damageable(enemy, 3));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed*3/4, 1));
        enemy->setDropGenerator(new DropGenerator(enemy, 400));

        return enemy;
    }

    return nullptr;
}

