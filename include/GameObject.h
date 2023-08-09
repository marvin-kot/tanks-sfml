#pragma once

#include "SpriteRenderer.h"
#include "GlobalConst.h"

class Controller;
class Shootable;
class Damageable;
class DropGenerator;
class Collectable;

class GameObject
{
    friend class OneShotAnimationRenderer;
    friend class SpawnController;
    friend class GrenadeCollectable;
    friend class TimerCollectable;
public:
    enum ObjectFlags
    {
        None = 0x0,
        BulletKillable = 0x1,
        TankPassable = 0x2,
        BulletPassable = 0x4,
        Bullet = 0x8,
        PiercingBullet = 0x10,
        MegaBullet = 0x20,
        Ice = 0x40,
        NPC = 0x80,
        Player = 0x100,
        Eagle = 0x200,
        Static = 0x400,
        BonusOnHit = 0x800,
        CollectableBonus = 0x1000
    };

private:
    std::string _type;
    int _id;

    int _parentId = -1;

    ObjectFlags _flags = None;

    bool _deleteme = false;

    Controller *_controller = nullptr;
    Shootable *_shootable = nullptr;
    Damageable *_damageable = nullptr;
    DropGenerator *_dropGenerator = nullptr;
    Collectable *_collectable = nullptr;

    globalTypes::Direction _direction;

    GameObject *_parentObject;
    int _x, _y;

    void assignUniqueId();

    int _lastUpdateFrame = 0;
public:
    SpriteRenderer *spriteRenderer = nullptr;
    GameObject *visualEffect = nullptr;
public:
    GameObject(std::string name);
    GameObject(GameObject *, std::string name);
    virtual ~GameObject();

    GameObject *getParentObject() const { return _parentObject; }
    void setFlags(ObjectFlags);
    void appendFlags(ObjectFlags);
    bool isFlagSet(ObjectFlags);

    inline void setParentId(int pid) { _parentId = pid; }

    virtual void draw();
    void hide(bool);
    void setPos(int x, int y);
    void move(int x, int y);

    sf::Vector2i position() const;
    void copyParentPosition(const GameObject * parent);

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;
    bool collidesWithAnyObject() const;
    std::vector<GameObject *> allCollisions() const;
    void updateOnCollision(GameObject *, bool& cancelMovement);
    void updateOnCollision(GameObject *);

    inline std::string type() const { return _type; }
    inline int id() const { return _id; }
    inline bool mustBeDeleted() const { return _deleteme; }

    void setController(Controller *);
    void setShootable(Shootable *);
    void setRenderer(SpriteRenderer *);
    void setDamageable(Damageable *);
    void setDropGenerator(DropGenerator *);
    void generateDrop();

    void setCollectable(Collectable *);
    void getCollectedBy(GameObject *collector);

    void setCurrentDirection(globalTypes::Direction);
    void setCurrentAnimation(std::string animName);
    void stopAnimation();
    void restartAnimation();

    void update();
    bool shoot();

    template <typename T>
    T *getComponent();
};


inline GameObject::ObjectFlags operator|(GameObject::ObjectFlags a, GameObject::ObjectFlags b)
{
    return static_cast<GameObject::ObjectFlags>(static_cast<int>(a) | static_cast<int>(b));
}
