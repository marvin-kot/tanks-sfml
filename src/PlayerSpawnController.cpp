
#include "Damageable.h"
#include "GameObject.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "Logger.h"
#include "ObjectsPool.h"

PlayerSpawnController::PlayerSpawnController(GameObject *parent, int lives, int powerLevel)
: Controller(parent, 0), _lives(lives), _initialPowerLevel(powerLevel)
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
    pc->setDamageable(new Damageable(pc, 1));
    // must be done after creating Damageable
    pc->setController(new PlayerController(pc));

    return pc;
}

void PlayerSpawnController::update()
{
    checkForGamePause();
    if (_pause) return;

    switch (_state) {
        case Waiting:
            if (ObjectsPool::playerObject == nullptr) {
                if (_lives < 1) {
                    _gameObject->markForDeletion();
                    return;
                }
                _initialPowerLevel = 0; // as player was killed, its power resets to 0
                globalVars::player1PowerLevel = 0;
                _state = Starting;
            }
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

                _clock.reset(true);
                _gameObject->hide(true);
                _state = Waiting;
            }
            break;
    }
}

void PlayerSpawnController::appendLife()
{
    _lives++;
    globalVars::player1Lives++;
}