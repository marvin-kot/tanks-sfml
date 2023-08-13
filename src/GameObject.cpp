#include "Collectable.h"
#include "Controller.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "Logger.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "Utils.h"

#include <iostream>


GameObject::GameObject(std::string type)
: _type(type)
{
    assignUniqueId();
}

GameObject::GameObject(GameObject *parent, std::string type)
: _parentObject(parent), _type(type)
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
    if (spriteRenderer)
        delete spriteRenderer;

    if (_controller)
        delete _controller;

    if (_shootable)
        delete _shootable;

    if (_damageable)
        delete _damageable;

    if (_dropGenerator)
        delete _dropGenerator;
}

void GameObject::setFlags(GameObject::ObjectFlags flags)
{
    _flags = flags;
}

void GameObject::appendFlags(GameObject::ObjectFlags f)
{
    _flags = _flags | f;
}

bool GameObject::isFlagSet(GameObject::ObjectFlags f)
{
    return (_flags & f) != 0;
}

void GameObject::draw(bool paused)
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
                _deleteme = true;
            return;
        }

        mappedX = mappedX - mapViewPort.left;
        mappedY = mappedY - mapViewPort.top;


        int screenX = mappedX + globalVars::gameViewPort.left;
        int screenY = mappedY + globalVars::gameViewPort.top;
        spriteRenderer->_sprite.setPosition(screenX, screenY);
        spriteRenderer->draw(paused);
    }
    else
        Logger::instance() << _type << "no renderer";

    if (visualEffect)
        visualEffect->draw(paused);
}

void GameObject::hide(bool val)
{
    Logger::instance() << "GameObject::hide" << val;
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
        return -1;

    if (spriteRenderer) {
        int mappedMoveX = x * globalConst::spriteScaleX;
        int mappedMoveY = y * globalConst::spriteScaleY;
        spriteRenderer->_sprite.move(mappedMoveX, mappedMoveY);
        _x += x, _y += y;

        GameObject *collider = nullptr;
        bool cancelMovement = false;
        sf::IntRect bb = boundingBox();// sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());

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
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
        _x += x, _y += y;
        return 1;
    }
}


void GameObject::updateOnCollision(GameObject *other, bool& cancelMovement)
{
    bool isBullet = isFlagSet(Bullet);

    if (other == nullptr) {
        // bullet just fled out of bounds
        if (isBullet)
            _deleteme = true;
        cancelMovement = true;
        return;
    }

    assert(other != nullptr);

    if (isBullet) {
        // just hit non-transparent target (and it's not its own creator)
        if (!other->isFlagSet(BulletPassable) && _parentId != other->id()) {
            _deleteme = true;
        }

        if (other->isFlagSet(Bullet))
            _deleteme = true;

        return;
    }

    assert(isBullet == false);

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
            }

            if (!friendlyFire) {
                if (_lastUpdateFrame == Utils::currentFrame)
                    return;
                _lastUpdateFrame = Utils::currentFrame;
                // TODO: variable bullet damage
                _damageable->takeDamage(1);
                if (_damageable->isDestroyed()) {
                    _deleteme = true;
                    cancelMovement = true;
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
}

bool GameObject::isOnIce() const
{
    for (GameObject *o : ObjectsPool::getAllObjects()) {
        if (this != o && collides(*o) && o->isFlagSet(GameObject::Ice))
            return true;
    }
    return false;
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

    return sf::IntRect(left+1, top+1, _w-2, _h-2);
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
        if (this != o && collides(*o))
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

void GameObject::setRenderer(SpriteRenderer *rndr)
{
    spriteRenderer = rndr;
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

    _deleteme = true;
}

void GameObject::setCurrentDirection(globalTypes::Direction dir)
{
    _direction = dir;
}

bool GameObject::shoot()
{
    if (_shootable)
        return _shootable->shoot(_direction);
    else
        return false;
}


void GameObject::update()
{
    if (_controller)
        _controller->update();
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

template<> PlayerShootable *GameObject::getComponent<PlayerShootable>()
{
    return dynamic_cast<PlayerShootable *>(_shootable);
}


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

template<> PlayerSpawnController *GameObject::getComponent<PlayerSpawnController>()
{
    return dynamic_cast<PlayerSpawnController *>(_controller);
}

template<> BulletController *GameObject::getComponent<BulletController>()
{
    return dynamic_cast<BulletController *>(_controller);
}
