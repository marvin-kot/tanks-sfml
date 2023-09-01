#include "GameObject.h"
#include "NpcTankController.h"
#include "Logger.h"
#include "ObjectsPool.h"
#include "SoundPlayer.h"
#include "Utils.h"

NpcTankController::NpcTankController(GameObject *parent, int spd, float timeoutSec)
: Controller(parent, spd), _actionTimeout(sf::seconds(timeoutSec))
{
    //_gameObject->setCurrentDirection(globalTypes::Down);
    _clock.reset(true);
}

bool NpcTankController::freezeIfGlobalFreezeActive()
{
    using namespace globalVars;
    if (globalTimeFreeze) {
        if (globalFreezeChronometer.getElapsedTime() < sf::seconds(globalFreezeTimeout)) {
            _currMoveX = _currMoveY = 0;
            _isMoving = false;
            _gameObject->stopAnimation();
            _gameObject->move(_currMoveX, _currMoveY);
            return true;
        }
        else
            globalTimeFreeze = false;
    }

    return false;
}

void NpcTankController::update()
{
    checkForGamePause();

    if (_pause)
        return;

    if (freezeIfGlobalFreezeActive())
        return;

    using namespace globalTypes;
    assert(_gameObject->direction() != globalTypes::Direction::Unknown );
    const auto oldDirection = _gameObject->direction();

    bool resetTimeout = tryToMove();

    if (decideIfToShoot(_gameObject->direction()))
        _gameObject->shoot();

    if (resetTimeout) {
        _clock.reset(true);
    }
}

int NpcTankController::trySqueeze()
{
    int moved = 1;
    if (_currMoveX == 0) {
        moved = _gameObject->move(1, _currMoveY);
        if (moved == 0) {
            moved = _gameObject->move(-1, _currMoveY);
        }

    } else if (_currMoveY == 0) {
        moved = _gameObject->move(_currMoveX, 1);
        if (moved == 0) {
            moved = _gameObject->move(_currMoveX, -1);
        }
    }

    return 0;
}

void NpcTankController::onDamaged()
{
    SpriteRenderer *renderer = _gameObject->getComponent<SpriteRenderer>();
    assert(renderer != nullptr);
    renderer->setOneFrameTintColor(sf::Color::Red);
}

bool NpcTankController::tryMoveToOneOfDirections(std::vector<globalTypes::Direction> directions)
{
    bool resetTimeout = false;
    int moved = -1; // TODO remove magic numbers
    int speed = moveSpeedForCurrentFrame();
    do {
        globalTypes::Direction dir = _gameObject->direction();
        if (dir == globalTypes::Unknown) {
            Logger::instance() << "ERROR: direction unknown " << _gameObject->type();
        }
        if (moved == 0 || _clock.getElapsedTime() > _actionTimeout) {
            resetTimeout = true;
            // change decision
            std::uniform_int_distribution<int> distribution(0, directions.size()-1);
            int index = distribution(Utils::generator);
            dir = directions[index];
            assert(dir != globalTypes::Direction::Unknown);
        }

        prepareMoveInDirection(dir, speed);

        moved = _gameObject->move(_currMoveX, _currMoveY);
        if (moved == 0) {
            // try same direction but +1/-1 pixes aside
            moved = trySqueeze();
            if (moved == 0) {
                // remove the direction from possible moves and try again
                directions.erase(std::remove(directions.begin(), directions.end(), dir), directions.end());
                resetTimeout = true;
            }
        }

    } while (resetTimeout && moved == 0 && !directions.empty());

    _isMoving = (moved>0);

    return resetTimeout;
}

TankRandomController::TankRandomController(GameObject *parent, int spd, float timeoutSec)
: NpcTankController(parent, spd, timeoutSec)
{}


bool TankRandomController::tryToMove()
{
    using namespace globalTypes;
    assert(_gameObject->direction() != Direction::Unknown );
    const auto oldDirection = _gameObject->direction();
    // add current direction to set to make moving in same direction more probable
    std::vector<Direction> possibleMoves = {Up, Left, Right, Down, oldDirection};

    return tryMoveToOneOfDirections(possibleMoves);
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


//////////////////

TankKamikazeController::TankKamikazeController(GameObject *parent, int spd, float timeoutSec)
: NpcTankController(parent, spd, timeoutSec) {}


bool TankKamikazeController::decideIfToShoot(globalTypes::Direction oldDir) const
{
    GameObject *target = nullptr;

    if (ObjectsPool::eagleObject && _gameObject->distanceTo(ObjectsPool::eagleObject) < 26)
        target = ObjectsPool::eagleObject;
    else if (ObjectsPool::playerObject && _gameObject->distanceTo(ObjectsPool::playerObject) < 20)
        target = ObjectsPool::playerObject;

    return (target != nullptr);
}


bool TankKamikazeController::tryToMove()
{
    GameObject *target = nullptr;

    if (ObjectsPool::eagleObject && _gameObject->distanceTo(ObjectsPool::eagleObject) < 80)
        target = ObjectsPool::eagleObject;
    else if (ObjectsPool::playerObject && _gameObject->distanceTo(ObjectsPool::playerObject) < 80)
        target = ObjectsPool::playerObject;

    using namespace globalTypes;
    std::vector<Direction> directions;

    for (int i=0; i<2; i++) {
        if (target) {
            if (target->position().x < _gameObject->position().x)
                directions.push_back(Direction::Left);
            if (target->position().y < _gameObject->position().y)
                directions.push_back(Direction::Up);
            if (target->position().x > _gameObject->position().x)
                directions.push_back(Direction::Right);
            if (target->position().y > _gameObject->position().y)
                directions.push_back(Direction::Down);
        }
    }

    directions.push_back(Direction::Left);
    directions.push_back(Direction::Up);
    directions.push_back(Direction::Right);
    directions.push_back(Direction::Down);


    return tryMoveToOneOfDirections(directions);
}


////////


TankBossController::TankBossController(GameObject *parent, int spd, float timeoutSec)
: NpcTankController(parent, spd, timeoutSec) {
    SoundPlayer::instance().bossAlive = true;
    SoundPlayer::instance().enqueueSound(SoundPlayer::BossTheme, true);

    _nextDirection = globalTypes::Direction::Unknown;

}


TankBossController::~TankBossController()
{
    ObjectsPool::bossObject = nullptr;
    SoundPlayer::instance().bossAlive = false;
    SoundPlayer::instance().enqueueSound(SoundPlayer::BossTheme, false);
}

bool TankBossController::decideIfToShoot(globalTypes::Direction oldDir) const
{
    // do not shoot right after turn
    if (_gameObject->direction() != oldDir)
        return false;

    using namespace globalTypes;

    sf::Vector2i pos = _gameObject->position();
    Direction dir = _gameObject->direction();

    int offsetX = (dir == Direction::Up || dir == Direction::Down) ? 4 : 0;
    int offsetY = (dir == Direction::Left || dir == Direction::Right) ? 4 : 0;

    bool okToShoot = false;

    {
        GameObject *hit = GameObject::linecastInDirection(_gameObject->id(), pos.x - offsetX, pos.y - offsetY, dir, 64, 512);

        if (hit != nullptr) {
            GameObject *closeHit = GameObject::linecastInDirection(_gameObject->id(), pos.x - offsetX, pos.y - offsetY, dir, 0, 50);
                if (closeHit)
                    return false;

            if (hit == ObjectsPool::playerObject || hit == ObjectsPool::eagleObject) {
                std::uniform_int_distribution<int> distribution(0, 4);
                int shotChance = distribution(Utils::generator);
                okToShoot = (shotChance == 0);
            }

            if (hit->type().rfind("brickWall", 0, 9) != std::string::npos) {
                std::uniform_int_distribution<int> distribution(0, 20);
                int shotChance = distribution(Utils::generator);
                if (shotChance == 0) okToShoot = true;
            }
        }
    }
    {
        GameObject *hit = GameObject::linecastInDirection(_gameObject->id(), pos.x + offsetX, pos.y + offsetY, dir, 64, 512);

        if (hit == nullptr)
            return false;

        GameObject *closeHit = GameObject::linecastInDirection(_gameObject->id(), pos.x + offsetX, pos.y + offsetY, dir, 0, 50);
        if (closeHit)
            return false;

        if (hit == ObjectsPool::playerObject || hit == ObjectsPool::eagleObject) {
            std::uniform_int_distribution<int> distribution(0, 4);
            int shotChance = distribution(Utils::generator);
            if (shotChance == 0) okToShoot = true;
        }

        if (hit->type().rfind("brickWall", 0, 9) != std::string::npos) {
            std::uniform_int_distribution<int> distribution(0, 20);
            int shotChance = distribution(Utils::generator);
            if (shotChance == 0) okToShoot = true;
        }
    }

    return okToShoot;

}


bool TankBossController::tryToMove()
{
    GameObject *target = nullptr;

    if (ObjectsPool::playerObject && _gameObject->distanceTo(ObjectsPool::playerObject) < 128)
        target = ObjectsPool::playerObject;
    else if (ObjectsPool::eagleObject && _gameObject->distanceTo(ObjectsPool::eagleObject) < 128)
        target = ObjectsPool::eagleObject;

    using namespace globalTypes;
    std::vector<Direction> directions;

    if (_nextDirection == globalTypes::Unknown) {
        for (int i=0; i<2; i++)
            if (target) {
                if (target->position().x < _gameObject->position().x)
                    directions.push_back(Direction::Left);
                if (target->position().y < _gameObject->position().y)
                    directions.push_back(Direction::Up);
                if (target->position().x > _gameObject->position().x)
                    directions.push_back(Direction::Right);
                if (target->position().y > _gameObject->position().y)
                    directions.push_back(Direction::Down);
            }

        directions.push_back(Direction::Left);
        directions.push_back(Direction::Up);
        directions.push_back(Direction::Right);
        directions.push_back(Direction::Down);
    } else {
        //globalTypes::Direction dir = _nextDirection.top();
        directions.push_back(_nextDirection);
    }


    bool resetTimeout = tryMoveToOneOfDirections(directions);
    if (resetTimeout)
        //_nextDirection.pop();
        _nextDirection = Direction::Unknown;

    return resetTimeout;
}

void TankBossController::onDamaged()
{
    NpcTankController::onDamaged();

    using namespace globalTypes;
    if (ObjectsPool::playerObject == nullptr)
        return;

    auto target = ObjectsPool::playerObject;

    if (target->position().x - _gameObject->position().x > 16)
        //_nextDirection.push(Direction::Right);
        _nextDirection = Direction::Right;
    if (target->position().x - _gameObject->position().x < -16)
        //_nextDirection.push(Direction::Left);
        _nextDirection = Direction::Left;
    if (target->position().y - _gameObject->position().y > 16)
        //_nextDirection.push(Direction::Down);
        _nextDirection = Direction::Down;
    if (target->position().y - _gameObject->position().y < -16)
        //_nextDirection.push(Direction::Up);
        _nextDirection = Direction::Up;
}
