#include "Controller.h"
#include "GameObject.h"

Controller::Controller(AnimatedGameObject *obj)
: _gameObject(obj) 
{}


PlayerController::PlayerController(AnimatedGameObject *obj)
: Controller(obj)
{}

void PlayerController::update()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        _gameObject->move(-moveSpeed, 0);
        _gameObject->setCurrentFrame("left");
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        _gameObject->move(0, -moveSpeed);
        _gameObject->setCurrentFrame("up");
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        _gameObject->move(moveSpeed, 0);
        _gameObject->setCurrentFrame("right");
    }
    else
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        _gameObject->move(0, moveSpeed);
        _gameObject->setCurrentFrame("down");
    }
}