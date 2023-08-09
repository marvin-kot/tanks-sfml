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

struct StarCollectable : public Collectable
{
    StarCollectable(GameObject *parent);
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