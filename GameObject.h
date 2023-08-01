#pragma once

#include "SpriteRenderer.h"

class GameObject
{
private:
    sf::RenderWindow& _gameWindow;
    std::string _name;

public:
    SpriteRenderer *spriteRenderer = nullptr;
public:
    GameObject(std::string name, sf::RenderWindow& w);
    virtual ~GameObject();
    
    virtual void draw();
    virtual void createSpriteRenderer(sf::Texture& tex);
    void setPos(int x, int y);
    void move(int x, int y);
};


class Controller;

class AnimatedGameObject : public GameObject
{
    Controller *_controller = nullptr;
public:
    AnimatedGameObject(std::string name, sf::RenderWindow& w);
    ~AnimatedGameObject();
    void setController(Controller *);
    void createSpriteRenderer(sf::Texture& tex) override;
    void assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy);
    void setCurrentFrame(std::string frameName);
    void update();

};
