#pragma once

#include "SpriteRenderer.h"
#include "GlobalConst.h"

class Controller;
class Shootable;

class GameObject
{
private:
    std::string _name;
    int id;

    int _parentId = -1;

    bool _killable = false;
    bool _passable = false;
    bool _killer = false;

    bool _deleteme = false;

    Controller *_controller = nullptr;
    Shootable *_shootable = nullptr;

    globalTypes::Direction _direction;

public:
    SpriteRenderer *spriteRenderer = nullptr;
public:
    GameObject(std::string name);
    void setBoolProperties(bool, bool, bool);
    virtual ~GameObject();

    inline void setParentId(int pid) { _parentId = pid; }

    virtual void draw();
    void createSpriteRenderer();
    void setPos(int x, int y);
    void move(int x, int y);

    sf::Vector2i position() const;
    void copyParentPosition(const GameObject * parent);

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;

    inline std::string name() const { return _name; }
    inline int Id() const { return id; }
    inline bool mustBeDeleted() const { return _deleteme; }

    void setController(Controller *);
    void setShootable(Shootable *);

    void setCurrentDirection(globalTypes::Direction);
    void setCurrentAnimation(std::string animName);
    void stopAnimation();
    void restartAnimation();

    void update();
    bool shoot();
};
