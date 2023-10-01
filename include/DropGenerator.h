#pragma once

#include <string>
#include <vector>

class GameObject;

class DropGenerator
{
private:
    GameObject *_gameObject;

    static GameObject *createObject(std::string parent);

    bool _used = false;
    int _xp;
    std::vector<std::string> _types;
public:
    DropGenerator(GameObject *parent, int xp);
    void setDropTypes(std::vector<std::string>);
    void placeRandomCollectable();
    void dropXp();

    bool isUsedOnce() const { return _used; }
};