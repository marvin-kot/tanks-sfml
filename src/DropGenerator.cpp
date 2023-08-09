#include "DropGenerator.h"

#include "GameObject.h"
#include "MapCreator.h"
#include "Utils.h"
#include "ObjectsPool.h"
#include "Collectable.h"

#include <random>

DropGenerator::DropGenerator(GameObject *parent)
: _gameObject(parent)
{}

void DropGenerator::placeRandomCollectable()
{
    std::uniform_int_distribution<int> x_distr(1, globalVars::mapSize.x-1);
    std::uniform_int_distribution<int> y_distr(1, globalVars::mapSize.y-1);

    std::vector<std::string> types = {
        "helmetCollectable",
        "timerCollectable",
        //"shovelCollectable",
        "starCollectable",
        "grenadeCollectable",
        //"tankCollectable"
    };

    int x = x_distr(Utils::generator);
    int y = y_distr(Utils::generator);

    std::uniform_int_distribution<int> type_distr(0, types.size()-1);
    int typeIndex = type_distr(Utils::generator);

    GameObject *collectable = createObject(types[typeIndex]);
    if (collectable) {
        collectable->setPos(x*64, y*64);
        ObjectsPool::addObject(collectable);
    }

    _used = true;
}

GameObject * DropGenerator::createObject(std::string type)
{
    if (!type.empty()) {
        GameObject *collectable = new GameObject(type);
        collectable->setFlags(GameObject::CollectableBonus | GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        collectable->setRenderer(new SpriteRenderer(collectable));

        Collectable *component = nullptr;
        if (type == "helmetCollectable")
            component = new HelmetCollectable(collectable);
        else if (type == "timerCollectable")
            component = new TimerCollectable(collectable);
        else if (type == "grenadeCollectable")
            component = new GrenadeCollectable(collectable);
        else if (type == "starCollectable")
            component = new StarCollectable(collectable);
        else
            component = new Collectable(collectable); // default collectable (useless)

        collectable->setCollectable(component);

        return collectable;
    }
    return nullptr;
}