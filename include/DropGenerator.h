#pragma once

#include <string>

class GameObject;

class DropGenerator
{
private:
    GameObject *_gameObject;

    static GameObject *createObject(std::string parent);

    bool _used = false;
    int _xp;
public:
    DropGenerator(GameObject *parent, int xp);
    void placeRandomCollectable();
    void dropXp();

    bool isUsedOnce() const { return _used; }
};