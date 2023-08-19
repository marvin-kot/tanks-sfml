#include "Controller.h"
#include "GlobalConst.h"
#include "GameObject.h"
#include "Utils.h"
#include "SoundPlayer.h"
#include "Shootable.h"
#include "Damageable.h"
#include "Logger.h"

#include <algorithm>


Controller::Controller(GameObject *parent, int spd)
: _gameObject(parent), _moveSpeed(spd)
{}

int Controller::moveSpeedForCurrentFrame()
{
    const float fSpeed = (float)_moveSpeed * Utils::lastFrameTime.asSeconds();
    int speed = std::floor(fSpeed);
    float fraction = fSpeed - speed;

    if (fraction >= 0.1 && fraction < 0.3) {
        if (Utils::currentFrame % 4 == 0)
            speed++;
    } else if (fraction >=0.3 && fraction < 0.4) {
        if (Utils::currentFrame % 3 == 0)
            speed++;
    } else if (fraction >=0.4 && fraction < 0.6) {
        if (Utils::currentFrame % 2 == 0)
            speed++;
    } else if (fraction >=0.6 && fraction < 0.8) {
        if (Utils::currentFrame % 3 != 0)
            speed++;
    } else if (fraction >= 0.8) {
        if (Utils::currentFrame % 4 != 0)
            speed++;
    }

    return speed;
}

void Controller::prepareMoveInDirection(globalTypes::Direction dir, int speed)
{
    assert(dir != globalTypes::Direction::Unknown);
    _gameObject->setCurrentDirection(dir);

    switch (dir) {
        case globalTypes::Left:
            _currMoveX = -speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("left");
            break;
        case globalTypes::Up:
            _currMoveY = -speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("up");
            break;
        case globalTypes::Right: // right
            _currMoveX = speed; _currMoveY = 0;
            _gameObject->setCurrentAnimation("right");
            break;
        case globalTypes::Down: // down
            _currMoveY = speed; _currMoveX = 0;
            _gameObject->setCurrentAnimation("down");
            break;
        default:
            _currMoveY = 0; _currMoveX = 0;
    }
}

void Controller::checkForGamePause()
{
    if (!_pause && globalVars::gameIsPaused) {
        _clock.pause();
        _pause = true;
    } else if (_pause && ! globalVars::gameIsPaused) {
        _clock.resume();
        _pause = false;
    }
}

/////

TankRandomController::TankRandomController(GameObject *parent, int spd, float timeoutSec)
: Controller(parent, spd), _actionTimeout(sf::seconds(timeoutSec))
{
    //_gameObject->setCurrentDirection(globalTypes::Down);
    _clock.reset(true);
}

void TankRandomController::update()
{
    checkForGamePause();

    if (_pause)
        return;

    {
        using namespace globalVars;
        if (globalTimeFreeze) {
            if (globalFreezeChronometer.getElapsedTime() < sf::seconds(globalFreezeTimeout)) {
                _currMoveX = _currMoveY = 0;
                _isMoving = false;
                _gameObject->stopAnimation();
                _gameObject->move(_currMoveX, _currMoveY);
                return;
            }
            else
                globalTimeFreeze = false;
        }
    }


    int moved = -1; // TODO remove magic numbers
    bool resetTimeout = false;
    int speed = moveSpeedForCurrentFrame();

    using namespace globalTypes;
    assert(_gameObject->direction() != globalTypes::Direction::Unknown );
    const auto oldDirection = _gameObject->direction();
    std::vector<Direction> possibleMoves = {Up, Left, Right, Down, oldDirection};
    // add current direction to set to make moving in same direction more probable
    do {
        globalTypes::Direction dir = _gameObject->direction();
        if (dir == globalTypes::Unknown) {
            Logger::instance() << "ERROR: direction unknown " << _gameObject->type();
        }
        if (moved == 0 || _clock.getElapsedTime() > _actionTimeout) {
            resetTimeout = true;
            // change decision
            std::uniform_int_distribution<int> distribution(0, possibleMoves.size()-1);
            int index = distribution(Utils::generator);
            dir = possibleMoves[index];
            assert(dir != globalTypes::Direction::Unknown);
        }

        prepareMoveInDirection(dir, speed);

        moved = _gameObject->move(_currMoveX, _currMoveY);
        if (moved == 0) {
            // try same direction but +1/-1 pixes aside
            moved = trySqueeze();
            if (moved == 0) {
                // remove the direction from possible moves and try again
                possibleMoves.erase(std::remove(possibleMoves.begin(), possibleMoves.end(), dir), possibleMoves.end());
                resetTimeout = true;
            }
        }

    } while (resetTimeout && moved == 0 && !possibleMoves.empty());

    _isMoving = (moved == 1);

    if (decideIfToShoot(oldDirection))
        _gameObject->shoot();

    if (resetTimeout) {
        _clock.reset(true);
    }
}

bool TankRandomController::decideIfToShoot(globalTypes::Direction oldDir) const
{
    // do not shoot right after turn
    if (_gameObject->direction() != oldDir)
        return false;

    GameObject *hit = _gameObject->linecastInCurrentDirection();

    if (hit == nullptr)
        return false;

    if (hit == ObjectsPool::playerObject || hit == ObjectsPool::eagleObject) {
        std::uniform_int_distribution<int> distribution(0, 10);
        int shotChance = distribution(Utils::generator);
        return (shotChance == 0);
    }

    if (hit->type().rfind("brickWall", 0, 9) != std::string::npos) {
        std::uniform_int_distribution<int> distribution(0, 20);
        int shotChance = distribution(Utils::generator);
        return (shotChance == 0);
    }

    return false;

}

int TankRandomController::trySqueeze()
{
    int moved = 1;
    if (_currMoveX == 0) {
        moved = _gameObject->move(2, _currMoveY);
        if (moved == 0)
            moved = _gameObject->move(-2, _currMoveY);
    } else if (_currMoveY == 0) {
        moved = _gameObject->move(_currMoveX, 2);
        if (moved == 0)
            moved = _gameObject->move(_currMoveX, -2);
    }

    return 0;
}

/////

BulletController::BulletController(GameObject *obj, globalTypes::Direction dir, int spd, int dmg)
: Controller(obj, spd), _direction(dir), _damage(dmg)
{
    _clock.reset(true);
}

void BulletController::update()
{

    checkForGamePause();
    if (_pause) return;

    if (_clock.getElapsedTime() > sf::milliseconds(globalConst::DefaultBulletLifetimeMs))
        _gameObject->markForDeletion();


    int speed = moveSpeedForCurrentFrame();
    assert(_direction != globalTypes::Direction::Unknown );
    prepareMoveInDirection(_direction, speed);
    _gameObject->move(_currMoveX, _currMoveY);

}


/////

