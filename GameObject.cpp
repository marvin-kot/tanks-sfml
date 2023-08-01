#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Utils.h"
#include "GlobalConst.h"

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
}

void GameObject::createSpriteRenderer()
{
    Logger::instance() << "createSpriteRenderer";
    spriteRenderer = new SpriteRenderer(_name);
}

void GameObject::draw()
{
    if (spriteRenderer)
        Utils::window.draw(spriteRenderer->_sprite);
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
    if (spriteRenderer) {
        sf::Vector2i oldPos = position();

        //sf::Vector2i newPos = pos + sf::Vector2i(x, y);
        spriteRenderer->_sprite.move(x, y);

        sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());

        bool isColliding = false;


        if (Utils::isOutOfBounds(thisBoundingBox))
            isColliding = true;
        else {
            for (GameObject *o : ObjectsPool::obstacles) {
                if (collides(*o)) {
                    isColliding = true;
                    break;
                }
            }
        }

        if (_name != "player" && collides(*ObjectsPool::playerObject))
                isColliding = true;
        
        for (GameObject *o : ObjectsPool::enemies) {
            if (id != o->id && collides(*o)) {
                isColliding = true;
                break;
            }
        }


        if (isColliding)
            spriteRenderer->_sprite.move(-x, -y);

    }
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

sf::Vector2i GameObject::position() const
{
    return sf::Vector2i(spriteRenderer->_sprite.getPosition());
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

// **** AnimatedGameObject

AnimatedGameObject::AnimatedGameObject(std::string name)
: GameObject(name)
{}


void AnimatedGameObject::setCurrentAnimation(std::string animName)
{
    if (spriteRenderer)
        spriteRenderer->setCurrentAnimation(animName);
    else
        Logger::instance() << "[ERROR] AnimatedGameObject - no spriteRenderer found";
}

void AnimatedGameObject::setController(Controller * ctrl)
{
    _controller = ctrl;
}

void AnimatedGameObject::update()
{
    if (_controller)
        _controller->update();
    else
        Logger::instance() << "[ERROR] AnimatedGameObject::update - no controller found";
}

AnimatedGameObject::~AnimatedGameObject()
{
    if (_controller)
        delete _controller;
}

