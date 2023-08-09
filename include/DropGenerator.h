#pragma once
#include <string>

class GameObject;
class DropGenerator
{
private:
    GameObject *_gameObject;

    static GameObject *createObject(std::string parent);

    bool _used = false;
public:
    DropGenerator(GameObject *parent);
    void placeRandomCollectable();

    bool isUsedOnce() const { return _used; }
};