#include "Collectable.h"
#include "Controller.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "EagleController.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Logger.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "SecondShootable.h"
#include "SoundPlayer.h"
#include "Utils.h"

#include "NetGameTypes.h"
#include <iostream>


GameObject::GameObject(std::string type)
: _type(type)
, _direction(globalTypes::Direction::Up)
, _turretRelativeDirection(globalTypes::Direction::Unknown)
, _parentObject(nullptr)
{
    assignUniqueId();
}

GameObject::GameObject(GameObject *parent, std::string type)
: _parentObject(parent), _type(type), _direction(globalTypes::Direction::Up)
{
    assignUniqueId();
}

void GameObject::assignUniqueId()
{
    static int count = 0;
    _id = count++;
}

GameObject::~GameObject()
{
    if (spriteRenderer) {
        delete spriteRenderer;
        spriteRenderer = nullptr;
    }

    if (_controller) {
        delete _controller;
        _controller = nullptr;
    }

    if (_secondShootable) {
        delete _secondShootable;
        _secondShootable = nullptr;
    }

    if (_shootable) {
        delete _shootable;
        _shootable = nullptr;
    }

    if (_damageable) {
        delete _damageable;
        _damageable = nullptr;
    }

    if (_dropGenerator)
        delete _dropGenerator;

    if (visualEffect)
        delete visualEffect;

    if (turret)
        delete turret;
}

void GameObject::setFlags(GameObject::ObjectFlags flags)
{
    _flags = flags;
}

void GameObject::appendFlags(GameObject::ObjectFlags f)
{
    _flags = _flags | f;
}

void GameObject::unsetFlags(GameObject::ObjectFlags flags)
{
    int newFlag = (int)flags;
    int oldFlag = (int)_flags;

    _flags = static_cast<GameObject::ObjectFlags>(oldFlag & ~newFlag);
}

bool GameObject::isFlagSet(GameObject::ObjectFlags f) const
{
    return ((int)_flags & (int)f) != 0;
}

//bool isAnyOfFlagsSet(std::vector<ObjectFlags>) const;

void GameObject::draw()
{
    if (_deleteme)
        return;

    using namespace globalVars;

    if (spriteRenderer) {
        int mappedX = _x * globalConst::spriteScaleX;
        int mappedY = _y * globalConst::spriteScaleY;

        // check if tile is outside view port
        if (mappedX < mapViewPort.left || mappedX > (mapViewPort.left+mapViewPort.width)
                || mappedY < mapViewPort.top || mappedY > (mapViewPort.top+mapViewPort.height)) {
            if (_type == "smallExplosion" || _type == "bigExplosion")
                markForDeletion();
            return;
        }

        mappedX = mappedX - mapViewPort.left;
        mappedY = mappedY - mapViewPort.top;


        int screenX = mappedX + globalVars::gameViewPort.left;
        int screenY = mappedY + globalVars::gameViewPort.top;
        spriteRenderer->setSpritePosition(screenX, screenY);
        spriteRenderer->draw();
    }
    else
        Logger::instance() << _type << "no renderer\n";

    if (turret) {
        turret->copyParentPosition(this);
        turret->draw();
    }

    if (visualEffect) {
        visualEffect->copyParentPosition(this);
        visualEffect->draw();
    }
}

bool GameObject::networkDraw(net::ThinGameObject& thin)
{
    if (_deleteme)
        return false;

    using namespace globalVars;
    int mappedX = _x * globalConst::spriteScaleX;
    int mappedY = _y * globalConst::spriteScaleY;

    // check if tile is outside view port
    if (mappedX < mapViewPort.left || mappedX > (mapViewPort.left+mapViewPort.width)
            || mappedY < mapViewPort.top || mappedY > (mapViewPort.top+mapViewPort.height)) {
        if (_type == "smallExplosion" || _type == "bigExplosion")
            markForDeletion();
        return false;
    }

    mappedX = mappedX - mapViewPort.left;
    mappedY = mappedY - mapViewPort.top;

    int screenX = mappedX + globalVars::gameViewPort.left;
    int screenY = mappedY + globalVars::gameViewPort.top;

    thin.x = screenX;
    thin.y = screenY;
    thin.id = _id;
    thin.flags = static_cast<uint16_t>(_flags);
    return spriteRenderer->networkDraw(thin);

    // TODO turret

    if (visualEffect) {
        net::ThinGameObject fx;
        fx.zorder = 5;
        if (visualEffect->networkDraw(fx))
            ObjectsPool::thinGameObjects[visualEffect->id()] = fx;
    }

    if (turret) {
        net::ThinGameObject turret;
        turret.zorder = 3;
        if (visualEffect->networkDraw(turret))
            ObjectsPool::thinGameObjects[visualEffect->id()] = turret;
    }
}

void GameObject::hide(bool val)
{
    if (spriteRenderer)
        spriteRenderer->hide(val);
}

void GameObject::setPosition(int x, int y)
{
    _x = x;
    _y = y;
}

void GameObject::offsetPosition(int offsetX, int offsetY)
{
    _x += offsetX;
    _y += offsetY;
}

sf::Vector2i GameObject::position() const
{
    return sf::Vector2i(_x, _y);
}

void GameObject::copyParentPosition(const GameObject * parent)
{
    auto pos = parent->position();
    _x = pos.x; _y = pos.y;
}



int GameObject::move(int x, int y)
{
    if (_deleteme)
        return 0;

    if (spriteRenderer) {
        int mappedMoveX = x * globalConst::spriteScaleX;
        int mappedMoveY = y * globalConst::spriteScaleY;
        spriteRenderer->_sprite.move(mappedMoveX, mappedMoveY);
        _x += x, _y += y;

        GameObject *collider = nullptr;
        bool cancelMovement = false;
        sf::IntRect bb = boundingBox();

        bool outOfBounds = bb.left < 0 || (bb.left+bb.width) > globalVars::mapSize.x || bb.top < 0 || (bb.top+bb.height) > globalVars::mapSize.y;

        if (outOfBounds) {
            updateOnCollision(collider, cancelMovement);
        }
        else {
            for (GameObject *o : ObjectsPool::getAllObjects()) {
                if (this != o && collides(*o)) {
                    collider = o;
                    updateOnCollision(collider, cancelMovement);
                    if (collider)
                        collider->updateOnCollision(this);
                }
            }
        }

        if (cancelMovement) {

            spriteRenderer->_sprite.move(-mappedMoveX, -mappedMoveY);
            _x -= x, _y -= y;
            moving = false;
            return 0;
        } else {
            moving = true;
            return 1;
        }
    }
    else {
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found\n";
        _x += x, _y += y;
        return 1;
    }
}


void GameObject::updateOnCollision(GameObject *other, bool& cancelMovement)
{
    if (_deleteme) return;
    bool isBullet = isFlagSet(Bullet);

    if (other == nullptr) {
        // bullet just fled out of bounds
        if (isBullet)
            markForDeletion();
        cancelMovement = true;
        return;
    }

    assert(other != nullptr);

    if (_parentId != -1 && (_parentId == other->_id || other->_parentId == _id))
        return;

    if (isBullet) {
        // to handle 1-turret-4-bullets situation
        if (other->isFlagSet(Bullet) && _parentId == other->_parentId)
            return;

        // just hit non-transparent target (and it's not its own creator)
        if (!other->isFlagSet(BulletPassable) && _parentId != other->id()) {
            // damage target
            if (isFlagSet(PiercingBullet)) {
                auto bullet = dynamic_cast<BulletController *>(_controller);
                assert(other != nullptr);
                if (other->isFlagSet(Bullet)) {
                    auto otherBullet = dynamic_cast<BulletController *>(other->_controller);
                    assert(otherBullet != nullptr);
                    if (bullet->damage() > otherBullet->damage())
                        bullet->loseDamage();
                    else {
                        markForDeletion();
                        return;
                    }
                }
            } else {
                markForDeletion();
                return;
            }
        }

        if (!isFlagSet(PiercingBullet) && other->isFlagSet(Bullet))// && _parentId != other->_parentId)
            markForDeletion();
        return;
    }

    // is Hit by bullet
    if (other->isFlagSet(Bullet)) {
        if (isFlagSet(BulletKillable) && _damageable) {
            bool friendlyFire = false;
            // check if its my own bullet
            if (id() == other->_parentId)
                friendlyFire = true;
            // check if I'm NPC and bullet is from another NPC (friendly fire)
            if (isFlagSet(NPC)) {
                GameObject *bulletAuthor = ObjectsPool::findNpcById(other->_parentId);
                if (bulletAuthor && bulletAuthor->isFlagSet(NPC))
                    friendlyFire = true;
                else if (bulletAuthor && bulletAuthor->isFlagSet(Player)) {
                    if (_damageable->isDestroyed()) {
                        auto pc = dynamic_cast<PlayerController *>(bulletAuthor->_controller);
                        assert(pc != nullptr);
                        pc->onKillEnemy(this);
                    }
                    else
                        SoundPlayer::instance().enqueueSound(SoundPlayer::DamageEnemy, true);
                }

            }

            if (!friendlyFire) {
                if (_lastUpdateFrame == Utils::currentFrame)
                    return;
                _lastUpdateFrame = Utils::currentFrame;
                auto bullet = dynamic_cast<BulletController *>(other->_controller);
                assert(bullet != nullptr);

                _damageable->takeDamage(bullet->damage());
                if (_damageable->isDestroyed()) {
                    markForDeletion();
                    cancelMovement = true;

                    if (isFlagSet(Static)) { // only brick wall has flags BulletKillable and Static
                        GameObject *bulletAuthor = ObjectsPool::findNpcById(other->_parentId);
                        if (bulletAuthor && bulletAuthor->isFlagSet(Player))
                            SoundPlayer::instance().enqueueSound(SoundPlayer::DestroyWall, true);
                    }
                }

                if (other->isFlagSet(PiercingBullet) && (bullet->loseDamage() < 1 || _damageable->isInvincible()))
                    other->markForDeletion();
            }
        }
        return;
    }

    // just run into wall or another tank
    if (!other->isFlagSet(TankPassable) && !isFlagSet(TankPassable)) {
        cancelMovement = true;
    }

    // check if player gathered a collectable
    if (isFlagSet(Player) && other->isFlagSet(CollectableBonus)) {
        other->getCollectedBy(this);
    }

    if (damage>0 && other->isFlagSet(BulletKillable) && other->_damageable && _parentId != other->_id) {
        other->_damageable->takeDamage(damage);
        if (other->_damageable->isDestroyed()) {
            other->markForDeletion();
            cancelMovement = false;
        } else {
            if (isFlagSet(Player)) {
                // if goal is not killed, stop the sequence
                auto pc = dynamic_cast<PlayerController *>(_controller);
                assert(pc != nullptr);
                pc->resetMoveStartTimer();
            }
        }
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);
    }

    if (_controller)
        _controller->onCollided(other);
}

bool GameObject::isOnIce() const
{
    for (GameObject *o : ObjectsPool::getAllObjects()) {
        if (this != o && collides(*o) && o->isFlagSet(GameObject::Ice))
            return true;
    }
    return false;
}

void GameObject::markForDeletion() {
    _deleteme = true;
    appendFlags(Delete);

    if (!ObjectsPool::setOfObjectsToDelete.contains(this)) {
        ObjectsPool::objectsToDelete.push(this);
        ObjectsPool::setOfObjectsToDelete.insert(this);
    }
}

void GameObject::updateOnCollision(GameObject *other)
{
    bool _;
    updateOnCollision(other, _);
}

sf::IntRect GameObject::boundingBox() const
{
    int left = _x - _w/2;
    int top = _y - _h/2;

    const int modif = (isFlagSet(Player|NPC|TankPassable|BulletPassable)) ? 1 : isFlagSet(CollectableBonus) ? -1 : 0;

    return sf::IntRect(left+modif, top+modif, _w-modif*2, _h-modif*2);
}

bool GameObject::collides(const GameObject& go) const
{
    auto thisBB = boundingBox();
    auto otherBB = go.boundingBox();

    using v2f = sf::Vector2i;

    bool intersects
            =  thisBB.contains(v2f(otherBB.left, otherBB.top))
            || thisBB.contains(v2f(otherBB.left + otherBB.width, otherBB.top))
            || thisBB.contains(v2f(otherBB.left + otherBB.width, otherBB.top + otherBB.height))
            || thisBB.contains(v2f(otherBB.left, otherBB.top + otherBB.height))
            || otherBB.contains(v2f(thisBB.left, thisBB.top))
            || otherBB.contains(v2f(thisBB.left + thisBB.width, thisBB.top))
            || otherBB.contains(v2f(thisBB.left + thisBB.width, thisBB.top + thisBB.height))
            || otherBB.contains(v2f(thisBB.left, thisBB.top + thisBB.height));

    return intersects;
}

bool GameObject::collidesWithAnyObject() const
{
    for (GameObject *o : ObjectsPool::getAllObjects()) {
        if (this != o && collides(*o) && !o->isFlagSet(TankPassable) && !o->isFlagSet(BulletPassable))
            return true;
    }
    return false;
}

std::vector<GameObject *> GameObject::allCollisions() const
{
    std::vector<GameObject *> result;
    for (GameObject *o : ObjectsPool::getAllObjects()) {
        if (this != o && collides(*o))
            result.push_back(o);
    }
    return result;
}

GameObject * GameObject::objectContainingPoint(std::unordered_set<GameObject *>& objectList, int id, int x, int y)
{
    if (ObjectsPool::playerObject!=nullptr && ObjectsPool::playerObject->boundingBox().contains(x, y))
        return ObjectsPool::playerObject;

    if (ObjectsPool::eagleObject!=nullptr && ObjectsPool::eagleObject->boundingBox().contains(x, y))
        return ObjectsPool::eagleObject;

    for (GameObject *o : objectList) {
        assert( o != nullptr );
        if (id == o->id())
            continue;
        if (o->mustBeDeleted())
            continue;

        if (o->boundingBox().contains(sf::Vector2i(x, y)))
            return o;
    }

    return nullptr;
}

GameObject *GameObject::linecastInDirection(int id, int startX, int startY, globalTypes::Direction direction, int minRange, int maxRange)
{
    using namespace globalTypes;
    using namespace globalVars;

    auto objectList = ObjectsPool::getObjectsByTypes(
        { "brickWall", "brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2", "concreteWall",
          "playerBase", "eagle", "npcBaseTank", "staticTurret",
          "npcFastTank", "npcArmorTank", "npcPowerTank", "npcGiantTank", "npcDoubleCannonArmorTank", "npcKamikazeTank",
          "greenTowerTrailer"
        });

    const int step = 4;
    switch (direction) {
        case Direction::Right:
            for (int x = startX + minRange; x < std::min(mapSize.x, startX + maxRange); x += step) {
                GameObject *obj = objectContainingPoint(objectList, id, x, startY);
                if (obj != nullptr)
                    return obj;
            } break;
        case Direction::Left:
            for (int x = startX - minRange; x > std::max(0, startX - maxRange) ; x -= step) {
                GameObject *obj = objectContainingPoint(objectList, id, x, startY);
                if (obj != nullptr)
                    return obj;
            } break;
        case Direction::Up:
            for (int y = startY - minRange; y > std::max(0, startY - maxRange) ; y -= step) {
                GameObject *obj = objectContainingPoint(objectList, id, startX, y);
                if (obj != nullptr)
                    return obj;
            } break;
        case Direction::Down:
            for (int y = startY + minRange; y < std::min(mapSize.y, startY + maxRange) ; y += step) {
                GameObject *obj = objectContainingPoint(objectList, id, startX, y);
                if (obj != nullptr)
                    return obj;
            } break;
    }

    return nullptr;
}

GameObject *GameObject::linecastInCurrentDirection(int minRange, int maxRange) const
{

    return linecastInDirection(_id, _x, _y, _direction, minRange, maxRange);
}


void GameObject::setCurrentAnimation(std::string animName)
{
    if (spriteRenderer)
        spriteRenderer->setCurrentAnimation(animName);
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

void GameObject::stopAnimation()
{
    if (spriteRenderer)
        spriteRenderer->playAnimation(false);
}

void GameObject::restartAnimation()
{
    if (spriteRenderer)
        spriteRenderer->playAnimation(true);
}

void GameObject::setController(Controller * ctrl)
{
    _controller = ctrl;
}


void GameObject::setShootable(Shootable * shtbl)
{
    _shootable = shtbl;
}

void GameObject::setSecondShootable(SecondShootable * shtbl)
{
    if (_secondShootable)
        delete _secondShootable;

    _secondShootable = shtbl;
}

void GameObject::setRenderer(SpriteRenderer *rndr, int order)
{
    spriteRenderer = rndr;
    _drawOrder = order;
}
void GameObject::setDamageable(Damageable *dmgbl)
{
    _damageable = dmgbl;
}

void GameObject::setDropGenerator(DropGenerator *dg)
{
    _dropGenerator = dg;
}

void GameObject::generateDrop()
{
    if (_dropGenerator && !_dropGenerator->isUsedOnce())
        _dropGenerator->placeRandomCollectable();
}

void GameObject::dropXp()
{
    // self exploded objects don't drop xp
    if (isFlagSet(Explosive))
        return;

    if (_dropGenerator && !_dropGenerator->isUsedOnce())
        _dropGenerator->dropXp();
}

void GameObject::setCollectable(Collectable *col)
{
    _collectable = col;
}

void GameObject::getCollectedBy(GameObject *collector)
{
    if (_collectable) {
        if (_collectable->onCollected(collector) == false)
            // leave this item uncollected
            return;
    }

    markForDeletion();
}

void GameObject::setCurrentDirection(globalTypes::Direction dir)
{
    _direction = dir;
    if (spriteRenderer)
        spriteRenderer->setCurrentDirection(dir);

    if (turret && turret->spriteRenderer) {

        if (_turretRelativeDirection == globalTypes::Direction::Unknown) {
            turret->spriteRenderer->setCurrentDirection(_direction);
        } else {
            turret->spriteRenderer->setCurrentDirection(turretAbsoluteDirection());
        }
    }
}



globalTypes::Direction GameObject::turretAbsoluteDirection() const {
    if (_turretRelativeDirection == globalTypes::Direction::Unknown)
        return _direction;

    int currDirValue = static_cast<int>(_direction);
    int dir = 0;
    switch (_turretRelativeDirection) {
        case globalTypes::Direction::Up:
            dir = currDirValue;
            break;
        case globalTypes::Direction::Left:
            dir = currDirValue + 1;
            if (dir > 4)
                dir = 1;
            break;
        case globalTypes::Direction::Right:
            dir = currDirValue - 1;
            if (dir < 1)
                dir += 4;
            break;
        case globalTypes::Direction::Down:
            dir = currDirValue + 2;
            if (dir > 4)
                dir -= 4;
            break;
    }

    return static_cast<globalTypes::Direction>(dir);
}

globalTypes::Direction GameObject::turretRelativeDirection(globalTypes::Direction absoluteDirection) const {
    assert(absoluteDirection != globalTypes::Direction::Unknown);

    int currDirValue = static_cast<int>(_direction) - 1;
    int absDirValue = static_cast<int>(absoluteDirection) - 1;

    int relDirValue = absDirValue - currDirValue;
    if (relDirValue < 0)
        relDirValue += 4;


    return static_cast<globalTypes::Direction>(relDirValue + 1);
}

bool GameObject::shoot()
{
    if (_shootable) {
        auto direction = _turretRelativeDirection == globalTypes::Direction::Unknown ? _direction : turretAbsoluteDirection();
        return _shootable->shoot(direction);
    }
    else
        return false;
}

bool GameObject::useSecondWeapon()
{
    if (_secondShootable)
        return _secondShootable->shoot(_direction);
    else
        return false;
}


net::ThinGameObject GameObject::update()
{
    if (_controller)
        _controller->update();
    else if (isFlagSet(GameObject::CollectableBonus))
        move(0, 0);

    if (_shootable)
        _shootable->reloadByTimeout();

    /*net::ThinGameObject thin;
    thin.id = _id;
    thin.x = _x;
    thin.x = _y;
    thin.flags = static_cast<uint16_t>(_flags);

    return thin*/

    return net::ThinGameObject();
}

int GameObject::distanceTo(GameObject *other)
{
    using namespace std;
    const long distX = abs(_x - other->_x);
    const long distY = abs(_y - other->_y);

    return static_cast<int>(sqrt(distX*distX + distY*distY));
}

template<typename T>
T *GameObject::getComponent()
{
    return nullptr;
}

template<> Controller * GameObject::getComponent<Controller>()
{
    return _controller;
}

template<> Shootable *GameObject::getComponent<Shootable>()
{
    return _shootable;
}
/*
template<> PlayerShootable *GameObject::getComponent<PlayerShootable>()
{
    return dynamic_cast<PlayerShootable *>(_shootable);
}*/


template<> Damageable *GameObject::getComponent<Damageable>()
{
    return _damageable;
}
template<> DropGenerator *GameObject::getComponent<DropGenerator>()
{
    return _dropGenerator;
}

template<> Collectable *GameObject::getComponent<Collectable>()
{
    return _collectable;
}

template<> SpriteRenderer *GameObject::getComponent<SpriteRenderer>()
{
    return spriteRenderer;
}

template<> PlayerController *GameObject::getComponent<PlayerController>()
{
             return dynamic_cast<PlayerController *>(_controller);
}

template<> EagleController *GameObject::getComponent<EagleController>()
{
    return dynamic_cast<EagleController *>(_controller);
}

template<> PlayerSpawnController *GameObject::getComponent<PlayerSpawnController>()
{
    return dynamic_cast<PlayerSpawnController *>(_controller);
}

template<> BulletController *GameObject::getComponent<BulletController>()
{
    return dynamic_cast<BulletController *>(_controller);
}
