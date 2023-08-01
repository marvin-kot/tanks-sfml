#pragma once

#include "SpriteRenderer.h"

class GameObject
{
private:
    std::string _name;
    int id;
public:
    SpriteRenderer *spriteRenderer = nullptr;
public:
    GameObject(std::string name);
    virtual ~GameObject();
    
    virtual void draw();
    void createSpriteRenderer();
    void setPos(int x, int y);
    void move(int x, int y);
    sf::Vector2i position() const;

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;
};


class Controller;

class AnimatedGameObject : public GameObject
{
    Controller *_controller = nullptr;
public:
    AnimatedGameObject(std::string name);
    ~AnimatedGameObject();
    void setController(Controller *);
    void setCurrentAnimation(std::string animName);
    void update();

};
