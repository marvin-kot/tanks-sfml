#pragma once

#include "SpriteRenderer.h"
#include "GlobalTypes.h"
#include <unordered_set>

class Controller;
class Shootable;
class SecondShootable;
class Damageable;
class DropGenerator;
class Collectable;

namespace net {
    class ThinGameObject;
}

class GameObject
{
    friend class OneShotAnimationRenderer;
    friend class SpriteRenderer;
    friend class SpawnController;
    friend class PlayerSpawnController;
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
        PowerBullet = 0x10,
        Ice = 0x20,
        NPC = 0x40,
        Player = 0x80,
        Eagle = 0x100,
        Static = 0x200,
        BonusOnHit = 0x400,
        CollectableBonus = 0x800,
        PlayerSpawner = 0x1000,
        PiercingBullet = 0x2000,
        Delete = 0x4000,
        Boss = 0x8000,
        Explosive = 0x10000,
        Landmine = 0x20000,
        OwnedByPlayer = 0x40000
    };

private:
    std::string _type;
    int _id;
    int _parentId = -1;

    ObjectFlags _flags = None;
    bool _deleteme = false;

    Shootable *_shootable = nullptr;
    Shootable *_secondShootable = nullptr;
    Damageable *_damageable = nullptr;
    DropGenerator *_dropGenerator = nullptr;
    Collectable *_collectable = nullptr;

    globalTypes::Direction _direction;
    globalTypes::Direction _turretRelativeDirection;

    GameObject *_parentObject = nullptr;
    int _x, _y;
    int _w, _h;

    void assignUniqueId();

    int _lastUpdateFrame = 0;

    int _drawOrder = 0;
public:
    SpriteRenderer *spriteRenderer = nullptr;
    GameObject *visualEffect = nullptr;
    GameObject *turret = nullptr;
    globalTypes::Direction turretDirection;

    bool moving = false;
    int damage = 0;
private:
    Controller *_controller = nullptr; // order matters
public:
    GameObject(std::string name);
    GameObject(GameObject *, std::string name);
    virtual ~GameObject();

    void setParent(GameObject *parent) { _parentObject = parent; }
    GameObject *getParentObject() const { return _parentObject; }
    void setFlags(ObjectFlags);
    void appendFlags(ObjectFlags);
    bool isFlagSet(ObjectFlags) const;
    void unsetFlags(ObjectFlags);

    inline void setParentId(int pid) { _parentId = pid; }
    inline int parentId() { return _parentId; }

    void draw();
    bool networkDraw(net::ThinGameObject&);
    void hide(bool);
    void setPosition(int x, int y);
    void offsetPosition(int x, int y);
    int move(int x, int y);
    bool isOnIce() const;
    void setSize(int w, int h) { _w = w; _h = h; }

    sf::Vector2i position() const;
    void copyParentPosition(const GameObject * parent);

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;
    bool collidesWithAnyObject() const;
    static GameObject *objectContainingPoint(std::unordered_set<GameObject *>& objectList, int id, int x, int y);
    static GameObject *linecastInDirection(int id, int x, int y, globalTypes::Direction dir, int minRange, int maxRange);
    GameObject *linecastInCurrentDirection(int minRange = 0, int maxRange = 512) const;
    std::vector<GameObject *> allCollisions() const;
    void updateOnCollision(GameObject *, bool& cancelMovement);
    void updateOnCollision(GameObject *);

    inline std::string type() const { return _type; }
    inline int id() const { return _id; }
    inline bool mustBeDeleted() const { return _deleteme; }
    void markForDeletion();
    inline int drawOrder() const { return _drawOrder; }

    void setController(Controller *);
    void setShootable(Shootable *);
    void setSecondShootable(SecondShootable *);
    void setRenderer(SpriteRenderer *, int order);
    void setDamageable(Damageable *);
    void setDropGenerator(DropGenerator *);
    void generateDrop();
    void dropXp();

    void setCollectable(Collectable *);
    void getCollectedBy(GameObject *collector);

    void setCurrentDirection(globalTypes::Direction);
    inline globalTypes::Direction direction() const {return _direction; }
    inline globalTypes::Direction turretAbsoluteDirection() const;
    globalTypes::Direction turretRelativeDirection(globalTypes::Direction absoluteDirection) const;
    void setTurretRelativeDirection(globalTypes::Direction dir) { _turretRelativeDirection = dir; }

    void setCurrentAnimation(std::string animName);
    void stopAnimation();
    void restartAnimation();

    int distanceTo(GameObject *);

    net::ThinGameObject update();
    bool shoot();
    bool useSecondWeapon();

    template <typename T>
    T *getComponent();
};


inline GameObject::ObjectFlags operator|(GameObject::ObjectFlags a, GameObject::ObjectFlags b)
{
    return static_cast<GameObject::ObjectFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline GameObject::ObjectFlags operator&(GameObject::ObjectFlags a, GameObject::ObjectFlags b)
{
    return static_cast<GameObject::ObjectFlags>(static_cast<int>(a) & static_cast<int>(b));
}