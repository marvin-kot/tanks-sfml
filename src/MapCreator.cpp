#include "MapCreator.h"

#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "Damageable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"

#include <string>


GameObject *MapCreator::buildObject(std::string type)
{
    if (type == "spawner_player") {
        Logger::instance() << "Creating player spawner...";
        GameObject *spawner = new GameObject("spawner_player");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        spawner->setController(new PlayerSpawnController(spawner, globalVars::player1Lives, globalVars::player1PowerLevel));

        return spawner;
    }

    if (type == "eagle") {
        GameObject *eagle = new GameObject("eagle");
        eagle->setFlags(GameObject::Eagle | GameObject::BulletKillable | GameObject::Static);
        eagle->setRenderer(new SpriteRenderer(eagle));
        eagle->setDamageable(new Damageable(eagle, 1));

        return eagle;
    }

    if (type == "spawner_BaseTank") {
        Logger::instance() << "Creating an enemy spawner...";
        GameObject *spawner = new GameObject("spawner_BaseTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        auto controller = new SpawnController(spawner, "npcBaseTank", 6, 10);
        controller->setBonusSpawnWithProbability(100);
        spawner->setController(controller);

        return spawner;
    }

    if (type == "spawner_FastTank") {
        Logger::instance() << "Creating an enemy spawner...";
        GameObject *spawner = new GameObject("spawner_FastTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        auto controller = new SpawnController(spawner, "npcFastTank", 7, 10);
        controller->setBonusSpawnWithProbability(100);
        spawner->setController(controller);

        return spawner;
    }

    if (type == "spawner_ArmorTank") {
        Logger::instance() << "Creating an enemy spawner...";
        GameObject *spawner = new GameObject("spawner_ArmorTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        auto controller = new SpawnController(spawner, "npcArmorTank", 8, 8);
        controller->setBonusSpawnWithProbability(100);
        spawner->setController(controller);

        return spawner;
    }

    if (type == "brickWall") {
        GameObject *wall = new GameObject("brickWall");
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }
    // wall parts
    if (type == "brickWall1x1" || type == "brickWall2x1" || type == "brickWall1x2" || type == "brickWall2x2") {
        GameObject *wall = new GameObject(type);
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }


    if (type == "concreteWall") {
        GameObject *wall = new GameObject("concreteWall");
        wall->setFlags(GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        auto damageable = new Damageable(wall, 0);
        damageable->makeInvincible(true);
        wall->setDamageable(damageable);

        return wall;
    }

    if (type == "tree") {
        GameObject *tree = new GameObject("tree");
        tree->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        tree->setRenderer(new SpriteRenderer(tree));

        return tree;
    }

    if (type == "water") {
        GameObject *water = new GameObject("water");
        water->setFlags(GameObject::BulletPassable | GameObject::Static);
        water->setRenderer(new SpriteRenderer(water));
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

    // WARNING: side effect - save map size globally
    globalVars::mapSize = sf::Vector2i(mapWidth(), mapHeight());
}

// ############ Custom Matrix Text ###################

MapCreatorFromCustomMatrixFile::MapCreatorFromCustomMatrixFile()
{
    charMap = {
            {'@', "spawner_player"},
            {'B', "spawner_BaseTank"},
            {'F', "spawner_FastTank"},
            {'P', "spawner_PowerTank"},
            {'A', "spawner_ArmorTank"},
            {'#', "brickWall"},
            {'*', "concreteWall"},
            {'!', "eagle"},
            {'&', "tree"},
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
                using namespace std;
                string objType = it->second;
                if (objType == "brickWall") {
                    vector<string> parts = {"brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2"};
                    int i=0;
                    for (auto part : parts) {
                        GameObject *object = MapCreator::buildObject(part);
                        assert(object != nullptr);
                        object->setPos(x*64 + 16 + (i%2)*32, y*64 + 16 + (i/2)*32);
                        ObjectsPool::addObject(object);
                        i++;
                    }
                    continue;
                }
                GameObject *object = MapCreator::buildObject(objType);
                if (object != nullptr) {
                    object->setPos(x*64 + 32, y*64 + 32);
                    ObjectsPool::addObject(object);
                    if (object->type() == "spawner_player") {
                        playerCreated = true;
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

        GameObject *object = MapCreator::buildObject(objType);
        if (object != nullptr) {
            object->setPos(x*64 + 32, y*64 + 32);
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