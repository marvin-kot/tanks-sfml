#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"
#include "GlobalConst.h"

#include <iostream>


GameObject::GameObject(std::string name, sf::RenderWindow& w)
: _gameWindow(w), _name(name)
{
}

GameObject::~GameObject()
{
    if (spriteRenderer)
        delete spriteRenderer;
    if (collider)
        delete collider;
}

void GameObject::createSpriteRenderer(sf::Texture& tex)
{
    Logger::instance() << "createSpriteRenderer";
    spriteRenderer = new SpriteRenderer(tex);
}

void GameObject::draw()
{
    if (spriteRenderer)
        _gameWindow.draw(spriteRenderer->_sprite);
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

void GameObject::createCollider(const sf::IntRect& rect)
{
    collider = new Collider(rect);
}
void GameObject::setPos(int x, int y)
{
    if (spriteRenderer)
        spriteRenderer->_sprite.setPosition(x, y);
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

bool isOutOfBounds(const sf::IntRect& rect) {
    return rect.left < 0
        || (rect.left+rect.width > globalConst::screen_w)
        || rect.top < 0
        || (rect.top+rect.height > globalConst::screen_h);
}

void GameObject::move(int x, int y)
{
    if (spriteRenderer) {
        sf::Vector2i oldPos = position();

        //sf::Vector2i newPos = pos + sf::Vector2i(x, y);
        spriteRenderer->_sprite.move(x, y);

        sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());

        if (isOutOfBounds(thisBoundingBox))
            spriteRenderer->_sprite.move(-x, -y);

    }
    else
        Logger::instance() << "[ERROR] GameObject - no spriteRenderer found";
}

sf::Vector2i GameObject::position() const
{
    return sf::Vector2i(spriteRenderer->_sprite.getPosition());
}

bool GameObject::collides(const GameObject& go)
{
    sf::IntRect thisBoundingBox = sf::IntRect(spriteRenderer->_sprite.getGlobalBounds());
    sf::IntRect otherBoundingBox = sf::IntRect(go.spriteRenderer->_sprite.getGlobalBounds());

    return thisBoundingBox.intersects(otherBoundingBox);
}

// **** AnimatedGameObject

AnimatedGameObject::AnimatedGameObject(std::string name, sf::RenderWindow& w)
: GameObject(name, w)
{}


void AnimatedGameObject::createSpriteRenderer(sf::Texture& tex)
{
    Logger::instance() << "AnimatedGameObject::createSpriteRenderer";
    spriteRenderer = new AnimatedSpriteRenderer(tex);
}

void AnimatedGameObject::assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy)
{
    if (spriteRenderer)
        spriteRenderer->assignAnimationFrame(id, ofx, ofy, sx, sy);
    else
        Logger::instance() << "[ERROR] AnimatedGameObject - no spriteRenderer found";
}

void AnimatedGameObject::setCurrentFrame(std::string frameName)
{
    if (spriteRenderer)
        spriteRenderer->showAnimationFrame(frameName);
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

