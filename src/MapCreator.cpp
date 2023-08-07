#include "MapCreator.h"

#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"

#include <string>


GameObject *MapCreator::buildObject(std::string type, int x, int y)
{
    if (type == "player") {
        Logger::instance() << "Creating player...";
        GameObject *pc = new GameObject("player");

        pc->setController(new PlayerController(pc));
        pc->setShootable(new Shootable(pc));
        pc->setFlags(GameObject::Player | GameObject::BulletKillable);
        pc->createSpriteRenderer();
        pc->setPos(x*64 + 32, y*64 + 32);
        pc->setCurrentDirection(globalTypes::Up);

        return pc;
    }

    if (type == "eagle") {
        GameObject *eagle = new GameObject("eagle");
        eagle->setFlags(GameObject::Eagle | GameObject::BulletKillable | GameObject::Static);
        eagle->createSpriteRenderer();
        eagle->setPos(x*64 + 32, y*64 + 32);

        return eagle;
    }

    if (type == "npcGreenArmoredTank") {
        Logger::instance() << "Creating an enemy...";
        GameObject *enemy = new GameObject("npcGreenArmoredTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->createSpriteRenderer();
        enemy->setController(new StupidController(enemy));

        enemy->setPos(x*64 + 32, y*64 + 32);

        return enemy;
    }

    if (type == "spawner_ArmoredTank") {
        Logger::instance() << "Creating an enemy spawner...";
        GameObject *spawner = new GameObject("spawner_ArmoredTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->spriteRenderer = new LoopAnimationSpriteRenderer(spawner, "spark");
        spawner->setController(new SpawnController(spawner, "npcGreenArmoredTank", 6, 10));

        spawner->setPos(x*64 + 32, y*64 + 32);

        return spawner;
    }

    if (type == "brickWall") {
        GameObject *wall = new GameObject("brickWall");
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->createSpriteRenderer();
        wall->setPos(x*64 + 32, y*64 + 32);

        return wall;
    }

    if (type == "concreteWall") {
        GameObject *wall = new GameObject("concreteWall");
        wall->setFlags(GameObject::Static);
        wall->createSpriteRenderer();
        wall->setPos(x*64 + 32, y*64 + 32);

        return wall;
    }

    if (type == "tree") {
        GameObject *tree = new GameObject("tree");
        tree->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        tree->createSpriteRenderer();
        tree->setPos(x*64 + 32, y*64 + 32);

        return tree;
    }

    if (type == "water") {
        GameObject *water = new GameObject("water");
        water->setFlags(GameObject::BulletPassable | GameObject::Static);
        water->createSpriteRenderer();
        water->setPos(x*64 + 32, y*64 + 32);
        water->setCurrentAnimation("default");

        return water;
    }

    return nullptr;
}

void MapCreator::setupScreenBordersBasedOnMapSize()
{
    assert(map_w > 0 && map_h > 0);
    globalVars::borderWidth = (globalConst::screen_w - mapWidth()*globalConst::spriteDisplaySizeX) / 2;
    globalVars::borderHeight = (globalConst::screen_h - mapHeight()*globalConst::spriteDisplaySizeY) / 2;
    globalVars::gameViewPort = sf::IntRect(globalVars::borderWidth, globalVars::borderHeight, globalConst::screen_w - globalVars::borderWidth*2, globalConst::screen_h - globalVars::borderHeight * 2);
}

// ############ Custom Matrix Text ###################

MapCreatorFromCustomMatrixFile::MapCreatorFromCustomMatrixFile()
{
    charMap = {
            {'@', "player"},
            {'x', "spawner_ArmoredTank"},
            {'#', "brickWall"},
            {'*', "concreteWall"},
            {'!', "eagle"},
            {'T', "tree"},
            {'~', "water"},
            {'_', "ice"}};
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
    // screen border properties must be set before building the map to place objects correctly
    setupScreenBordersBasedOnMapSize();
    bool playerCreated = false;
    for (int y = 0; y < map_h; y++) {
        for (int x = 0; x < map_w; x++) {
            char tile = mapString[y*map_w + x];
            auto it = charMap.find(tile);
            if (it != charMap.end()) {
                std::string objType = it->second;
                GameObject *object = MapCreator::buildObject(objType, x, y);
                if (object != nullptr) {
                    ObjectsPool::addObject(object);
                    if (object->type() == "player") {
                        playerCreated = true;
                        ObjectsPool::playerObject = object;
                    }
                }
            }
        }
    }

    if (!playerCreated) {
        Logger::instance() << "[Error] No player created";
        return -1;
    }
    return 0;
}


// ############## JSON ###############################
int MapCreatorFromJson::parseMapFile(std::string jsonName)
{
    std::ifstream f(jsonName);
    data = json::parse(f);
    map_w = 16; map_h = 10;

    return 0;
}

int MapCreatorFromJson::buildMapFromData()
{
    setupScreenBordersBasedOnMapSize();
    bool playerCreated = false;
    for (json::iterator it = data.begin(); it != data.end(); ++it) {
        json j = *it;

        std::string objType = j["type"];
        int x = j["x"];
        int y = j["y"];

        GameObject *object = MapCreator::buildObject(objType, x, y);
        if (object != nullptr) {
            ObjectsPool::addObject(object);
            if (object->type() == "player") {
                playerCreated = true;
                ObjectsPool::playerObject = object;
            }
        }
    }


    if (!playerCreated) {
        Logger::instance() << "[Error] No player created";
        return -1;
    }

    return 0;
}