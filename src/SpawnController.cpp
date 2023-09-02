#include "NpcTankController.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "GameObject.h"
#include "ObjectsPool.h"
#include "Shootable.h"
#include "Utils.h"

SpawnController::SpawnController(GameObject *parent, std::string type, int delay, int timeout, int quantity)
: Controller(parent, 0)
, _spawnableType(type)
, _startSpawnDelay(sf::seconds(delay))
, _spawnTimeout(sf::seconds(timeout))
, _quantity(quantity)
{
    _clock.reset(true);
    _gameObject->hide(true);
    _state = StartDelay;
}


void SpawnController::update()
{
    checkForGamePause();
    if (_pause) return;

    if (_quantity < 1) {
        _gameObject->markForDeletion();
        return;
    }

    switch (_state) {
        case StartDelay:
            if (globalVars::globalChronometer.getElapsedTime() > _startSpawnDelay)
                _state = Starting;
            break;
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

                    if (newNpc->isFlagSet(GameObject::Boss))
                        ObjectsPool::bossObject = newNpc;

                    _quantity--;
                }

                _clock.reset(true);
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
        enemy->setShootable(Shootable::createDefaultEnemyShootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 0));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed, 1));
        enemy->setDropGenerator(new DropGenerator(enemy, 100));

        return enemy;
    }

    if (type == "npcFastTank") {
        GameObject *enemy = new GameObject("npcFastTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::HalvedTimeoutMs));
        enemy->setShootable(Shootable::createDefaultEnemyShootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 0));
        enemy->setController(new TankRandomController(enemy, globalConst::FastEnemySpeed, 0.75));
        enemy->setDropGenerator(new DropGenerator(enemy, 200));

        return enemy;
    }

    if (type == "npcArmorTank") {
        GameObject *enemy = new GameObject("npcArmorTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        enemy->setShootable(Shootable::createDefaultEnemyShootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 3));
        enemy->setController(new TankRandomController(enemy, globalConst::SlowEnemySpeed, 1.5));
        enemy->setDropGenerator(new DropGenerator(enemy, 400));

        return enemy;
    }

    if (type == "npcPowerTank") {
        GameObject *enemy = new GameObject("npcPowerTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        Shootable *shootable = Shootable::createDefaultEnemyShootable(enemy);
        shootable->setDamage(2);
        shootable->setBulletSpeed(globalConst::DoubleEnemyBulletSpeed);
        enemy->setShootable(shootable);
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 1));
        enemy->setController(new TankRandomController(enemy, globalConst::DefaultEnemySpeed, 1.5));
        enemy->setDropGenerator(new DropGenerator(enemy, 500));

        return enemy;
    }

    if (type == "npcGiantTank") {
        GameObject *enemy = new GameObject("npcGiantTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        Shootable *shootable = Shootable::createDoubleRocketShootable(enemy);
        shootable->setDamage(2);
        enemy->damage = 1; // has contact damage
        enemy->setShootable(shootable);
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable | GameObject::Boss);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 80));
        enemy->setController(new TankBossController(enemy, globalConst::SlowEnemySpeed*3/4, 2));
        enemy->setDropGenerator(new DropGenerator(enemy, 1000));

        return enemy;
    }

    if (type == "npcDoubleCannonArmorTank") {
        GameObject *enemy = new GameObject("npcDoubleCannonArmorTank");
        //enemy->setShootable(new Shootable(enemy, globalConst::DefaultTimeoutMs));
        Shootable *shootable = new DoubleShootable(enemy);
        shootable->setDamage(1);

        enemy->setShootable(shootable);
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 5));
        enemy->setController(new TankRandomController(enemy, globalConst::SlowEnemySpeed, 1.7));
        enemy->setDropGenerator(new DropGenerator(enemy, 600));

        return enemy;
    }

    if (type == "npcKamikazeTank") {
        GameObject *enemy = new GameObject("npcKamikazeTank");

        enemy->setShootable(new KamikazeShootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->setRenderer(new SpriteRenderer(enemy), 2);
        enemy->setDamageable(new Damageable(enemy, 0));
        enemy->setController(new TankKamikazeController(enemy, globalConst::FastEnemySpeed, 0.7));
        enemy->setDropGenerator(new DropGenerator(enemy, 300));

        return enemy;
    }

    return nullptr;
}

