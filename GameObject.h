#pragma once

#include "SpriteRenderer.h"
#include "GlobalConst.h"

class Controller;
class Shootable;

class GameObject
{
public:
    enum ObjectFlags
    {
        None = 0,
        BulletKillable = 1,
        TankPassable = 2,
        BulletPassable = 4,
        Bullet = 8,
        PiercingBullet = 16,
        MegaBullet = 32,
        Ice = 64,
        NPC = 128,
        Player = 256,
        Eagle = 384
    };

private:
    std::string _name;
    int id;

    int _parentId = -1;

    ObjectFlags _flags = None;

    bool _deleteme = false;

    Controller *_controller = nullptr;
    Shootable *_shootable = nullptr;

    globalTypes::Direction _direction;

public:
    SpriteRenderer *spriteRenderer = nullptr;
public:
    GameObject(std::string name);
    virtual ~GameObject();

    void setFlags(ObjectFlags);
    bool isFlagSet(ObjectFlags);

    inline void setParentId(int pid) { _parentId = pid; }

    virtual void draw();
    void createSpriteRenderer();
    void setPos(int x, int y);
    void move(int x, int y);

    sf::Vector2i position() const;
    void copyParentPosition(const GameObject * parent);

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;
    void updateOnCollision(GameObject *, bool& cancelMovement);
    void updateOnCollision(GameObject *);

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


inline GameObject::ObjectFlags operator|(GameObject::ObjectFlags a, GameObject::ObjectFlags b)
{
    return static_cast<GameObject::ObjectFlags>(static_cast<int>(a) | static_cast<int>(b));
}
