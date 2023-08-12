#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Utils.h"
#include "Shootable.h"
#include "Damageable.h"
#include "DropGenerator.h"
#include "Collectable.h"

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

void GameObject::draw()
{
    if (_deleteme)
        return;

    if (spriteRenderer)
        spriteRenderer->draw();
    else
        Logger::instance() << _type << "no renderer";

    if (visualEffect)
        visualEffect->draw();
}

void GameObject::hide(bool val)
{
    Logger::instance() << "GameObject::hide" << val;
    if (spriteRenderer)
        spriteRenderer->hide(val);
}

void GameObject::setPos(int x, int y)
{
    if (spriteRenderer) {
        int mappedX = x + globalVars::gameViewPort.left;
        int mappedY = y + globalVars::gameViewPort.top;
        spriteRenderer->_sprite.setPosition(mappedX, mappedY);
    }
    _x = x + globalVars::gameViewPort.left;
    _y = y + globalVars::gameViewPort.top;
}


int GameObject::move(int x, int y)
{
    if (_deleteme)
        return -1;

    if (spriteRenderer) {
        spriteRenderer->_sprite.move(x, y);
        _x += x, _y += y;

        GameObject *collider = nullptr;
        bool cancelMovement = false;
        sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());

        if (Utils::isOutOfBounds(thisBoundingBox)) {
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
            spriteRenderer->_sprite.move(-x, -y);
            _x -= x, _y -= y;
            return 0;
        } else
            return 1;
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
        if (!other->isFlagSet(BulletPassable) && !isFlagSet(PiercingBullet) && _parentId != other->id()) {
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

void GameObject::updateOnCollision(GameObject *other)
{
    bool _;
    updateOnCollision(other, _);
}

sf::Vector2i GameObject::position() const
{
    if (spriteRenderer)
        return sf::Vector2i(spriteRenderer->_sprite.getPosition());
    else
        return sf::Vector2i(_x, _y);
}

void GameObject::copyParentPosition(const GameObject * parent)
{
    auto pos = parent->position();
    spriteRenderer->_sprite.setPosition(pos.x, pos.y);
    _x = pos.x; _y = pos.y;
}

sf::IntRect GameObject::boundingBox() const
{

    sf::IntRect thisBoundingBox;

    if (spriteRenderer) {
        thisBoundingBox =  sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());
        // reduct rect
        thisBoundingBox.left += 4;
        thisBoundingBox.top += 4;
        thisBoundingBox.width -= 8;
        thisBoundingBox.height -= 8;
    }
    else
        thisBoundingBox =  sf::IntRect(_x, _y, 0, 0);

    return thisBoundingBox;
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
