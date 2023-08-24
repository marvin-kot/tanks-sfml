
#include "Damageable.h"
#include "GameObject.h"
#include "PlayerController.h"
#include "PlayerUpgrade.h"
#include "Shootable.h"
#include "Logger.h"
#include "ObjectsPool.h"

PlayerSpawnController::PlayerSpawnController(GameObject *parent, int lives, int powerLevel)
: Controller(parent, 0)
{
    _gameObject->hide(true);
    _state = Starting;
}

PlayerSpawnController::~PlayerSpawnController()
{
}

GameObject * PlayerSpawnController::createObject()
{
    Logger::instance() << "Creating player...\n";
    GameObject *pc = new GameObject("player");
    pc->setShootable(new PlayerShootable(pc, 0));
    pc->setFlags(GameObject::Player | GameObject::BulletKillable);
    pc->setRenderer(new SpriteRenderer(pc));
    pc->setDamageable(new Damageable(pc, globalConst::DefaultPlayerProtection));
    // must be done after creating Damageable
    pc->setController(new PlayerController(pc));

    return pc;
}

void PlayerSpawnController::update()
{
    checkForGamePause();
    if (_pause) return;

    switch (_state) {
        case Starting: {
            _state = Spawning;
        }   break;
        case Waiting:
            if (ObjectsPool::playerObject == nullptr) {
                if (globalVars::player1Lives < 1) {
                    _gameObject->markForDeletion();
                    return;
                }
                globalVars::player1PowerLevel = 0;
                _state = Spawning;
            }
            break;
        case Spawning:
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
                    controller->setTemporaryInvincibility(3000);
                }

                _clock.reset(true);
                _gameObject->hide(true);
                _state = Waiting;
            }
            break;
    }
}

void PlayerSpawnController::appendLife()
{
    globalVars::player1Lives++;
}