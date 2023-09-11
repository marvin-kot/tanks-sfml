#pragma once

class GameObject;

class Collectable
{
    GameObject *_gameObject;

public:
    Collectable(GameObject *parent);
    virtual void onCollected(GameObject *collector);
};

struct GrenadeCollectable : public Collectable
{
    GrenadeCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};

struct TimerCollectable : public Collectable
{
    TimerCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};

struct HelmetCollectable : public Collectable
{
    HelmetCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};

struct TankCollectable : public Collectable
{
    TankCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};

struct XpCollectable : public Collectable
{

    XpCollectable(GameObject *parent, int value);
    void onCollected(GameObject *collector) override;

    int value() const { return _value; }
    void setValue(int val) { _value = val; }

private:
    int _value;
};

struct AmmoCollectable : public Collectable
{
    AmmoCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};

struct RepairCollectable : public Collectable
{
    RepairCollectable(GameObject *parent);
    void onCollected(GameObject *collector) override;
};