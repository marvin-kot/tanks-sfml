#pragma once

#include "SpriteRenderer.h"
#include "GlobalTypes.h"

class Controller;
class Shootable;
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
        Delete = 0x4000
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
    int _w, _h;

    void assignUniqueId();

    int _lastUpdateFrame = 0;
public:
    SpriteRenderer *spriteRenderer = nullptr;
    GameObject *visualEffect = nullptr;
    bool moving = false;
public:
    GameObject(std::string name);
    GameObject(GameObject *, std::string name);
    virtual ~GameObject();

    void setParent(GameObject *parent) { _parentObject = parent; }
    GameObject *getParentObject() const { return _parentObject; }
    void setFlags(ObjectFlags);
    void appendFlags(ObjectFlags);
    bool isFlagSet(ObjectFlags) const;

    inline void setParentId(int pid) { _parentId = pid; }
    inline int parentId() { return _parentId; }

    void draw();
    bool networkDraw(net::ThinGameObject&);
    void hide(bool);
    void setPosition(int x, int y);
    int move(int x, int y);
    bool isOnIce() const;
    void setSize(int w, int h) { _w = w; _h = h; }

    sf::Vector2i position() const;
    void copyParentPosition(const GameObject * parent);

    sf::IntRect boundingBox() const;
    bool collides(const GameObject& go) const;
    bool collidesWithAnyObject() const;
    GameObject *objectContainingPoint(int id, int x, int y) const;
    GameObject *linecastInCurrentDirection() const;
    std::vector<GameObject *> allCollisions() const;
    void updateOnCollision(GameObject *, bool& cancelMovement);
    void updateOnCollision(GameObject *);

    inline std::string type() const { return _type; }
    inline int id() const { return _id; }
    inline bool mustBeDeleted() const { return _deleteme; }
    inline void markForDeletion() { _deleteme = true; appendFlags(Delete); }

    void setController(Controller *);
    void setShootable(Shootable *);
    void setRenderer(SpriteRenderer *);
    void setDamageable(Damageable *);
    void setDropGenerator(DropGenerator *);
    void generateDrop();
    void dropXp();

    void setCollectable(Collectable *);
    void getCollectedBy(GameObject *collector);

    void setCurrentDirection(globalTypes::Direction);
    inline globalTypes::Direction direction() const {return _direction; }
    void setCurrentAnimation(std::string animName);
    void stopAnimation();
    void restartAnimation();

    net::ThinGameObject update();
    bool shoot();

    template <typename T>
    T *getComponent();
};


inline GameObject::ObjectFlags operator|(GameObject::ObjectFlags a, GameObject::ObjectFlags b)
{
    return static_cast<GameObject::ObjectFlags>(static_cast<int>(a) | static_cast<int>(b));
}
