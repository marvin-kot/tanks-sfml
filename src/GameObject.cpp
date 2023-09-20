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
#include "SoundPlayer.h"
#include "Utils.h"

#include "NetGameTypes.h"
#include <iostream>


GameObject::GameObject(std::string type)
: _type(type)
, _direction(globalTypes::Direction::Up)
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
        spriteRenderer->_sprite.setPosition(screenX, screenY);
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
    Logger::instance() << "GameObject::hide " << val << "\n";
    if (spriteRenderer)
        spriteRenderer->hide(val);
}

void GameObject::setPosition(int x, int y)
{
    _x = x;
    _y = y;
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
        // to handle 1-turre-4-bullets situation
        if (other->isFlagSet(Bullet) && _parentId == other->_parentId)
            return;

        // just hit non-transparent target (and it's not its own creator)
        if (!other->isFlagSet(BulletPassable) && _parentId != other->id()) {
            // damage target
            /*if (other->isFlagSet(BulletKillable) && other->_damageable != nullptr) {
                other->_damageable->takeDamage(damage);
                if (other->_damageable->isDestroyed()) {
                    other->markForDeletion();
                }
            }*/

            if (isFlagSet(PiercingBullet)) {
                auto bullet = dynamic_cast<BulletController *>(_controller);
                if (other->isFlagSet(BulletKillable)) {
                    if (bullet->loseDamage() < 1)
                        markForDeletion();
                } else if (other->isFlagSet(Bullet)) {
                    auto otherBullet = dynamic_cast<BulletController *>(other->_controller);
                    if (bullet->damage() > otherBullet->damage())
                        bullet->loseDamage();
                    else
                        markForDeletion();
                } else if (!other->isFlagSet(BulletPassable))
                    markForDeletion(); // if hit concrete block
            } else {
                markForDeletion();
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
                    if (_damageable->isDestroyed())
                        dynamic_cast<PlayerController *>(bulletAuthor->_controller)->onKillEnemy(this);
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
            }
        }
        return;
    }

    // just run into wall or another tank
    if (!other->isFlagSet(TankPassable)) {
        cancelMovement = true;
    }

    // check if player gathered a collectable
    if (isFlagSet(Player) && other->isFlagSet(CollectableBonus)) {
        other->getCollectedBy(this);
    }

    if (damage>0 && other->isFlagSet(BulletKillable) && other->_damageable) {
        other->_damageable->takeDamage(damage);
        if (other->_damageable->isDestroyed()) {
            other->markForDeletion();
            cancelMovement = false;
        } else {
            if (isFlagSet(Player))
                // if goal is not killed, stop the sequence
                dynamic_cast<PlayerController *>(_controller)->resetMoveStartTimer();
        }
        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);
    }

    // merge colliding xp points to reduce number of objects on map
    if (isFlagSet(CollectableBonus) && other->isFlagSet(CollectableBonus)) {
        XpCollectable *thisXp = dynamic_cast<XpCollectable *>(_collectable);
        if (thisXp == nullptr)
            return;
        XpCollectable *thatXp = dynamic_cast<XpCollectable *>(other->_collectable);
        if (thatXp == nullptr)
            return;
        int sum = thisXp->value() + thatXp->value();
        if (sum > 1000 || (sum % 100 != 0))
            return; // 900 is maximum xp collectable...

        // update value
        thisXp->setValue(sum);
        std::string newType;
        switch (sum) {
            case 200: newType = "200xp"; break;
            case 300: newType = "300xp"; break;
            case 400: newType = "400xp"; break;
            case 500: newType = "500xp"; break;
            case 600: newType = "600xp"; break;
            case 700: newType = "700xp"; break;
            case 800: newType = "800xp"; break;
            case 900: newType = "900xp"; break;
            case 1000: newType = "1000xp"; break;
            default:
                return;
        }

        spriteRenderer->setNewObjectType(newType);

        // delete second object
        other->markForDeletion();

    }
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
    ObjectsPool::objectsToDelete.push(this);
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

    auto objectList = ObjectsPool::getObjectsByTypes({ "brickWall", "brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2", "concreteWall" });

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
    if (_collectable)
        _collectable->onCollected(collector);

    markForDeletion();
}

void GameObject::setCurrentDirection(globalTypes::Direction dir)
{
    _direction = dir;
    if (spriteRenderer)
        spriteRenderer->setCurrentDirection(dir);

    if (turret && turret->spriteRenderer)
        turret->spriteRenderer->setCurrentDirection(dir);
}

bool GameObject::shoot()
{
    if (_shootable)
        return _shootable->shoot(_direction);
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
