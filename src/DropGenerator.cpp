#include "DropGenerator.h"

#include "GameObject.h"
#include "MapCreator.h"
#include "Utils.h"
#include "ObjectsPool.h"
#include "Collectable.h"
#include "Controller.h"

#include <random>
#include <map>

DropGenerator::DropGenerator(GameObject *parent, int xp)
: _gameObject(parent), _xp(xp)
{}

void DropGenerator::placeRandomCollectable()
{
    std::uniform_int_distribution<int> x_distr(1, (globalVars::mapSize.x/globalConst::spriteOriginalSizeX)-1);
    std::uniform_int_distribution<int> y_distr(1, (globalVars::mapSize.y/globalConst::spriteOriginalSizeY)-1);

    const std::vector<std::string> types = {
        //"helmetCollectable",
        //"timerCollectable",
        //"shovelCollectable",
        //"starCollectable",
        //"grenadeCollectable",
        //"tankCollectable",
        "ammoCollectable",
        "ammoCollectable",
        "ammoCollectable",
        "toolsCollectable"
    };

    //int x = x_distr(Utils::generator);
    //int y = y_distr(Utils::generator);

    std::uniform_int_distribution<int> type_distr(0, types.size()-1);
    int typeIndex = type_distr(Utils::generator);

    GameObject *collectable = createObject(types[typeIndex]);
    if (collectable) {
        //collectable->setPosition(x*globalConst::spriteOriginalSizeX, y*globalConst::spriteOriginalSizeX);
        collectable->copyParentPosition(_gameObject);
        ObjectsPool::addObject(collectable);
    }

    _used = true;
}

void DropGenerator::dropXp()
{
    const std::map<int, std::string> xpTypes = {
        {100, "100xp"},
        {200, "200xp"},
        {300, "300xp"},
        {400, "400xp"},
        {500, "500xp"},
        {600, "600xp"},
        {700, "700xp"},
        {800, "800xp"},
        {900, "900xp"},
        {1000, "1000xp"},
    };
    GameObject *collectable = createObject(xpTypes.at(_xp));
    if (collectable) {
        collectable->copyParentPosition(_gameObject);
        ObjectsPool::addObject(collectable);
    }
}

GameObject * DropGenerator::createObject(std::string type)
{
    if (!type.empty()) {
        GameObject *collectable = new GameObject(type);
        collectable->setFlags(GameObject::CollectableBonus | GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        collectable->setRenderer(new SpriteRenderer(collectable), 4);

        Collectable *component = nullptr;
        if (type == "helmetCollectable")
            component = new HelmetCollectable(collectable);
        else if (type == "timerCollectable")
            component = new TimerCollectable(collectable);
        else if (type == "grenadeCollectable")
            component = new GrenadeCollectable(collectable);
        else if (type == "tankCollectable")
            component = new TankCollectable(collectable);
        else if (type == "ammoCollectable")
            component = new AmmoCollectable(collectable);
        else if (type == "toolsCollectable")
            component = new RepairCollectable(collectable);
        else if (type == "100xp")
            component = new XpCollectable(collectable, 100);
        else if (type == "200xp")
            component = new XpCollectable(collectable, 200);
        else if (type == "300xp")
            component = new XpCollectable(collectable, 300);
        else if (type == "400xp")
            component = new XpCollectable(collectable, 400);
        else if (type == "500xp")
            component = new XpCollectable(collectable, 500);
        else if (type == "600xp")
            component = new XpCollectable(collectable, 600);
        else if (type == "700xp")
            component = new XpCollectable(collectable, 700);
        else if (type == "800xp")
            component = new XpCollectable(collectable, 800);
        else if (type == "900xp")
            component = new XpCollectable(collectable, 900);
        else if (type == "1000xp")
            component = new XpCollectable(collectable, 1000);
        else
            component = new Collectable(collectable); // default collectable (useless)

        collectable->setCollectable(component);
        collectable->setController(new CollectableXpController(collectable));

        return collectable;
    }
    return nullptr;
}

