#include "MapCreator.h"

#include "GameObject.h"
#include "ObjectsPool.h"

#include <string>


GameObject *MapCreator::buildObject(std::string type, int x, int y)
{
    if (type == "brickWall") {
        GameObject *wall = new GameObject("brickWall");
        wall->setFlags(GameObject::BulletKillable);
        wall->createSpriteRenderer();
        wall->setPos(x*64 + 32, y*64 + 32);

        return wall;
    }

    if (type == "concreteWall") {
        GameObject *wall = new GameObject("concreteWall");
        wall->createSpriteRenderer();
        wall->setPos(x*64 + 32, y*64 + 32);

        return wall;
    }

    if (type == "tree") {
        GameObject *tree = new GameObject("tree");
        tree->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        tree->createSpriteRenderer();
        tree->setPos(x*64 + 32, y*64 + 32);

        return tree;
    }

    if (type == "water") {
        GameObject *water = new GameObject("water");
        water->setFlags(GameObject::BulletPassable);
        water->createSpriteRenderer();
        water->setPos(x*64 + 32, y*64 + 32);
        water->setCurrentAnimation("default");

        return water;
    }

    if (type == "eagle") {
        GameObject *eagle = new GameObject("eagle");
        eagle->setFlags(GameObject::Eagle | GameObject::BulletKillable);
        eagle->createSpriteRenderer();
        eagle->setPos(x*64 + 32, y*64 + 32);

        return eagle;
    }



    return nullptr;
}

// ############ Custom Matrix Text ###################

MapCreatorFromCustomMatrixFile::MapCreatorFromCustomMatrixFile()
{
    charMap = {
            {'b', "brickWall"},
            {'c', "concreteWall"},
            {'e', "eagle"},
            {'t', "tree"},
            {'w', "water"},
            {'i', "ice"}};
}

int MapCreatorFromCustomMatrixFile::parseMapFile(std::string fileName)
{
    // file format:
    // line 1: width, height
    // next lines: map

    std::ifstream f(fileName);
    std::string line;
    std::getline(f, line);

    std::istringstream iss(line);
    iss >> map_w >> map_h;
    Logger::instance() << "Read map size: " << map_w << map_h;

    // read the rest of the file
    while (std::getline(f, line))
    {
        mapString.append(line);
    }

    return 0;
}

int MapCreatorFromCustomMatrixFile::buildMapFromData()
{
    for (int y = 0; y < map_h; y++) {
        for (int x = 0; x < map_w; x++) {
            char tile = mapString[y*map_w + x];
            auto it = charMap.find(tile);
            if (it != charMap.end()) {
                std::string objType = it->second;
                GameObject *object = MapCreator::buildObject(objType, x, y);
                if (object != nullptr)
                    ObjectsPool::obstacles.insert(object);
            }
        }
    }

    return 0;
}


// ############## JSON ###############################
int MapCreatorFromJson::parseMapFile(std::string jsonName)
{
    std::ifstream f(jsonName);
    data = json::parse(f);

    return 0;
}

int MapCreatorFromJson::buildMapFromData()
{
    for (json::iterator it = data.begin(); it != data.end(); ++it) {
        json j = *it;

        std::string objType = j["type"];
        int x = j["x"];
        int y = j["y"];

        GameObject *object = MapCreator::buildObject(objType, x, y);
        if (object != nullptr)
            ObjectsPool::obstacles.insert(object);
    }

    return 0;
}