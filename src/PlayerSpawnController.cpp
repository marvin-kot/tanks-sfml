
#include "Damageable.h"
#include "GameObject.h"
#include "PlayerController.h"
#include "PlayerUpgrade.h"
#include "Shootable.h"
#include "Logger.h"
#include "ObjectsPool.h"
#include "SecondShootable.h"

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
    GameObject *pc = new GameObject("playerBase");
    pc->setShootable(Shootable::createDefaultPlayerShootable(pc));
    //TEMP //pc->setSecondShootable(new JezekSetter(pc));
    pc->setFlags(GameObject::Player | GameObject::BulletKillable);
    pc->setRenderer(new SpriteRenderer(pc), 2);
    pc->setDamageable(new Damageable(pc, globalConst::DefaultPlayerProtection));

    pc->turret = new GameObject("playerTurret");
    pc->turret->setParent(pc);
    pc->turret->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
    pc->turret->setRenderer(new SpriteRenderer(pc->turret), 3);

    // must be done after creating Damageable and turret
    auto controller = new PlayerController(pc);
    pc->setController(controller);
    controller->applyPerks(); // must be done after setting controller

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

                    // to test: create a trailer
                    /*GameObject *trailer = new GameObject("greenTowerTrailer");

                    trailer->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
                    trailer->setRenderer(new SpriteRenderer(trailer), 2);
                    trailer->setDamageable(new Damageable(trailer, 0));
                    trailer->setController(new TrailerController(trailer, controller));
                    trailer->copyParentPosition(_gameObject);
                    trailer->move(0, 28);

                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    controller->moveTrailBuffer.push(0, -4);
                    ObjectsPool::addObject(trailer);*/
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

GameObject *PlayerSpawnController::onDestroyed()
{
    ObjectsPool::playerSpawnerObject = nullptr;
    return Controller::onDestroyed();
}