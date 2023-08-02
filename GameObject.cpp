#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Utils.h"
#include "Shootable.h"

#include <iostream>


GameObject::GameObject(std::string name)
: _name(name)
{
    static int count = 0;

    id = count++;
}

GameObject::~GameObject()
{
    if (spriteRenderer)
        delete spriteRenderer;

    if (_controller)
        delete _controller;

    if (_shootable)
        delete _shootable;
}

void GameObject::setBoolProperties(bool klbl, bool psbl, bool klr)
{
    _killable = klbl;
    _passable = psbl;
    _killer = klr;
}

void GameObject::createSpriteRenderer()
{
    Logger::instance() << "createSpriteRenderer";
    spriteRenderer = new SpriteRenderer(_name);
}

void GameObject::draw()
{
    if (_deleteme)
        return;

    if (spriteRenderer)
        spriteRenderer->draw();
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

void GameObject::setPos(int x, int y)
{
    if (spriteRenderer) {
        int mappedX = x + globalConst::gameViewPort.left;
        int mappedY = y + globalConst::gameViewPort.top;
        spriteRenderer->_sprite.setPosition(mappedX, mappedY);
    }
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}



void GameObject::move(int x, int y)
{
    if (_deleteme)
        return;

    if (spriteRenderer) {
        sf::Vector2i oldPos = position();

        //sf::Vector2i newPos = pos + sf::Vector2i(x, y);
        spriteRenderer->_sprite.move(x, y);

        sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());

        bool isColliding = false;

        GameObject *collider = nullptr;


        if (Utils::isOutOfBounds(thisBoundingBox))
            isColliding = true;
        else {
            for (GameObject *o : ObjectsPool::obstacles) {
                if (collides(*o)) {
                    collider = o;
                    isColliding = true;
                    break;
                }
            }
        }

        if (_name != "player" && collides(*ObjectsPool::playerObject)) {
                collider = ObjectsPool::playerObject;
                isColliding = true;
        }

        for (GameObject *o : ObjectsPool::enemies) {
            if (id != o->id && collides(*o)) {
                collider = o;
                isColliding = true;
                break;
            }
        }

        if (isColliding) {
            if (_killer && collider == nullptr) {
                // out of bounds. TODO: how to delete it from the pool?
                _deleteme = true;
                return;
            }

            if (_killer && collider && collider->_killable) {
                // check for owner
                if (_parentId != collider->Id()) {
                    ObjectsPool::kill(collider);
                    _deleteme = true;
                }

                return;
            }

            if (_killable && collider && collider->_killer) {
                if (id != collider->_parentId) {
                    ObjectsPool::kill(collider);
                    _deleteme = true;
                }

                return;
            }

            if (_killer && collider && !collider->_passable)
            {
                _deleteme = true;
            }

            spriteRenderer->_sprite.move(-x, -y);
        }

    }
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

sf::Vector2i GameObject::position() const
{
    return sf::Vector2i(spriteRenderer->_sprite.getPosition());
}

void GameObject::copyParentPosition(const GameObject * parent)
{
    auto pos = parent->position();
    spriteRenderer->_sprite.setPosition(pos.x, pos.y);
}

sf::IntRect GameObject::boundingBox() const
{
    sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());
    // reduct rect
    thisBoundingBox.left += 2;
    thisBoundingBox.top += 2;
    thisBoundingBox.width -= 4;
    thisBoundingBox.height -= 4;

    return thisBoundingBox;
}

bool GameObject::collides(const GameObject& go) const
{
    return boundingBox().intersects(go.boundingBox());
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
    else
        Logger::instance() << "[ERROR] GameObject::update - no controller found";
}

